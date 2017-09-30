// *****************************************************************************
/*!
  \file      src/Inciter/Partitioner.h
  \copyright 2012-2015, J. Bakosi, 2016-2017, Los Alamos National Security, LLC.
  \brief     Charm++ chare partitioner group used to perform mesh partitioning
  \details   Charm++ chare partitioner group used to perform mesh partitioning.
    The implementation uses the Charm++ runtime system and is fully
    asynchronous, overlapping computation, communication as well as I/O. The
    algorithm utilizes the structured dagger (SDAG) Charm++ functionality. The
    high-level overview of the algorithm structure and how it interfaces with
    Charm++ is discussed in the Charm++ interface file
    src/Inciter/partitioner.ci.

    #### Call graph ####
    The following is a directed acyclic graph (DAG) that outlines the
    asynchronous algorithm implemented in this class The detailed discussion of
    the algorithm is given in the Charm++ interface file partitioner.ci,
    which also repeats the graph below using ASCII graphics. On the DAG orange
    fills denote global synchronization points, orange frames with white fill
    are partial synchronization points that overlap with other tasks, and dashed
    lines are potential shortcuts that allow jumping over some of the task-graph
    under some circumstances. See the detailed discussion in partitioner.ci.
    \dot
    digraph "Partitioner SDAG" {
      rankdir="LR";
      node [shape=record, fontname=Helvetica, fontsize=10];
      Own [ label="Own" tooltip="owned nodes reordered"
             URL="\ref inciter::Partitioner::reorder"];
      Req [ label="Req" tooltip="nodes requested"
             URL="\ref inciter::Partitioner::request"];
      Pre [ label="Pre" tooltip="start preparing node IDs"
            URL="\ref inciter::Partitioner::prepare" color="#e6851c"];
      Ord [ label="Ord" tooltip="Node IDs reordered"
            URL="\ref inciter::Partitioner::reordered" color="#e6851c"];
      Low [ label="Low" tooltip="lower bound received"
             URL="\ref inciter::Partitioner::lower"];
      Upp [ label="Upp" tooltip="upper bound computed"
             URL="\ref inciter::Partitioner::bounds"];
      Par [ label="Par" tooltip="partitioners participated"
             URL="\ref inciter::Partitioner::neworder"];
      Cre [ label="Cre" tooltip="create workers"
             URL="\ref inciter::Partitioner::create" color="#e6851c"];
      Own -> Pre [ style="solid" ];
      Req -> Pre [ style="solid" ];
      Pre -> Ord [ style="solid" ];
      Ord -> Low [ style="solid" ];
      Ord -> Upp [ style="solid" ];
      Ord -> Par [ style="solid" ];
      Low -> Cre [ style="solid" ];
      Upp -> Cre [ style="solid" ];
      Par -> Cre [ style="solid" ];
    }
    \enddot
    \include Inciter/partitioner.ci
*/
// *****************************************************************************
#ifndef Partitioner_h
#define Partitioner_h

#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <numeric>

#include "ExodusIIMeshReader.h"
#include "ContainerUtil.h"
#include "ZoltanInterOp.h"
#include "Inciter/InputDeck/InputDeck.h"
#include "Options/PartitioningAlgorithm.h"
#include "Solver.h"
#include "DerivedData.h"
#include "UnsMesh.h"
#include "AMR/mesh_adapter.h"

#include "NoWarning/partitioner.decl.h"

namespace inciter {

extern ctr::InputDeck g_inputdeck;
extern CkReduction::reducerType NodesMerger;

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wundefined-func-template"
#endif

//! Partitioner Charm++ chare group class
//! \details Instantiations of Partitioner comprise a processor aware Charm++
//!   chare group. When instantiated, a new object is created on each PE and not
//!   more (as opposed to individual chares or chare array object elements). See
//!   also the Charm++ interface file partitioner.ci.
template< class WorkerProxy >
class Partitioner : public CBase_Partitioner< WorkerProxy > {

  #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-parameter"
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
  #elif defined(STRICT_GNUC)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  #elif defined(__INTEL_COMPILER)
    #pragma warning( push )
    #pragma warning( disable: 1478 )
  #endif
  // Include Charm++ SDAG code. See http://charm.cs.illinois.edu/manuals/html/
  // charm++/manual.html, Sec. "Structured Control Flow: Structured Dagger".
  Partitioner_SDAG_CODE
  #if defined(__clang__)
    #pragma clang diagnostic pop
  #elif defined(STRICT_GNUC)
    #pragma GCC diagnostic pop
  #elif defined(__INTEL_COMPILER)
    #pragma warning( pop )
  #endif

  private:
    using Group = CBase_Partitioner< WorkerProxy >;

  public:
    //! Constructor
    //! \param[in] cb Charm++ callbacks
    //! \param[in] host Host Charm++ proxy we are being called from
    //! \param[in] worker Worker Charm++ proxy we spawn PDE work to
    //! \param[in] solver Linear system solver proxy
    Partitioner( const std::vector< CkCallback >& cb,
                 const CProxy_Transporter& host,
                 const WorkerProxy& worker,
                 const tk::CProxy_Solver< WorkerProxy >& solver ) :
      __dep(),
      m_cb( cb[0], cb[1], cb[2], cb[3], cb[4], cb[5] ),
      m_host( host ),
      m_worker( worker ),
      m_solver( solver ),
      m_npe( 0 ),
      m_reqNodes(),
      m_reqEdges(),
      m_start( 0 ),
      m_noffset( 0 ),
      m_nquery( 0 ),
      m_tetinpoel(),
      m_gelemid(),
      m_centroid(),
      m_nchare( 0 ),
      m_lower( 0 ),
      m_upper( 0 ),
      m_ncomm(),
      m_ecomm(),
      m_ncommunication(),
      m_ecommunication(),
      m_nodeset(),
      m_edgeset(),
      m_linnodes(),
      m_linedges(),
      m_chinpoel(),
      m_chfilenodes(),
      m_chedgenodes(),
      m_cost( 0.0 ),
      m_nodechares(),
      m_edgechares(),
      m_msum(),
      m_msumed()
    {
      tk::ExodusIIMeshReader
        er( g_inputdeck.get< tag::cmd, tag::io, tag::input >() );
      // Read our contiguously-numbered chunk of the mesh graph from file
      readGraph( er );
      // If a geometric partitioner is selected, compute element centroid
      // coordinates
      const auto alg = g_inputdeck.get< tag::selected, tag::partitioner >();
      if ( tk::ctr::PartitioningAlgorithm().geometric(alg) )
        computeCentroids( er );
      else
        Group::contribute( m_cb.get< tag::part >() );
    }

    //! Partition the computational mesh
    //! \param[in] nchare Number of parts the mesh will be partitioned into
    void partition( int nchare ) {
      m_nchare = nchare;
      const auto alg = g_inputdeck.get< tag::selected, tag::partitioner >();
      const auto che = tk::zoltan::geomPartMesh( alg,
                                                 m_centroid,
                                                 m_gelemid,
                                                 m_tetinpoel.size()/4,
                                                 nchare );
      // send progress report to host
//       if ( g_inputdeck.get< tag::cmd, tag::feedback >() )
//         m_host.pepartitioned();
      Assert( che.size() == m_gelemid.size(), "Size of ownership array does "
              "not equal the number of mesh graph elements" );
      // Construct global mesh node ids for each chare and distribute
      distribute( chareNodes(che) );
      // Free storage of element connectivity, element centroids, and element
      // IDs as they are no longer needed after the mesh partitioning.
      tk::destroy( m_gelemid );
      tk::destroy( m_centroid );
    }

    //! Receive number of uniquely assigned global mesh node IDs from lower PEs
    //! \details This function computes the offset each PE will need to start
    //!   assigning its new node IDs from (for those nodes that are not assigned
    //!   new IDs by any PEs with lower indices). The offset for a PE is the
    //!   offset for the previous PE plus the number of node IDs the previous PE
    //!   (uniquely) assigns new IDs for minus the number of node IDs the
    //!   previous PE receives from others (lower PEs). This is computed here in
    //!   a parallel/distributed fashion by each PE sending its number of node
    //!   IDs (that it uniquely assigns) to all PEs. Note that each PE would
    //!   only need to send this information to higher PEs, but instead this
    //!   function is called in a broadcast fashion, because that is more
    //!   efficient than individual calls to only the higher PEs. Therefore when
    //!   computing the offsets, we only count the lower PEs. When this is done,
    //!   we have the precise communication map as well as the start offset on
    //!   all PEs and so we can start the distributed global mesh node ID
    //!   reordering.
    void offset( int p, std::size_t u ) {
      if (p < CkMyPe()) m_start += u;
      if (++m_noffset == static_cast<std::size_t>(CkNumPes())) reorder();
    }

    //! Request new global node IDs for old node IDs
    //! \param[in] p PE request coming from and to which we send new IDs to
    //! \param[in] nd Set of old node IDs whose new IDs are requested
    void request( int p, const std::unordered_set< std::size_t >& nd ) {
      // Queue up requesting PE and node IDs
      m_reqNodes.push_back( { p, nd } );
      // Trigger SDAG wait signaling that node IDs have been requested from us
      nodes_requested_complete();
    }

    //! Request new global node IDs for edges
    //! \param[in] p PE request coming from and to which we send new IDs to
    //! \param[in] ed Set of edges whose new IDs are requested
    void request( int p, const tk::UnsMesh::Edges& ed ) {
      // Queue up requesting PE and node IDs
      m_reqEdges.push_back( { p, ed } );
      // Trigger SDAG wait signaling that node IDs have been requested from us
      nodes_requested_complete();
    }

    //! Receive new (reordered) global node IDs
    //! \param[in] nd Map associating new to old node IDs
    void neworder( const std::unordered_map< std::size_t, std::size_t >& nd ) {
      // Signal to the runtime system that we have participated in reordering
      participated_complete();
      // Store new node IDs associated to old ones
      for (const auto& p : nd) m_linnodes[ p.first ] = p.second;
      // If all our nodes have new IDs assigned, signal that to the runtime
      if (m_linnodes.size() == m_nodeset.size()) nodesreorder_complete();
    }

    //! Receive new global node IDs associated to edge-nodes
    //! \param[in] ed Map associating node IDs to edges
    void neworder( const tk::UnsMesh::EdgeNodes& ed ) {
      // Signal to the runtime system that we have participated in reordering
      participated_complete();
      // Store node IDs associated to edge
      for (const auto& e : ed) m_linedges[ e.first ] = e.second;
      // If all our edges have new IDs assigned, signal that to the runtime
      if (m_linedges.size() == m_edgeset.size()) edgesreorder_complete();
    }

    //! Receive mesh node IDs associated to chares we own
    //! \param[in] n Mesh node indices associated to chare IDs
    //! \param[in] frompe PE call coming from
    void add( int frompe,
              const std::unordered_map< int, std::vector< std::size_t > >& n )
    {
      for (const auto& c : n) {
        Assert( pe(c.first) == CkMyPe(), "PE " + std::to_string(CkMyPe()) +
                " received a chareid-nodeidx-vector pair whose chare it does"
                " not own" );
        auto& inpoel = m_chinpoel[ c.first ];
        inpoel.insert( end(inpoel), begin(c.second), end(c.second) );
      }
      Group::thisProxy[ frompe ].recv();
    }

    //! Acknowledge received node IDs
    void recv() {
      if (--m_npe == 0) Group::contribute( m_cb.get< tag::distributed >() );
    }

    //! Prepare owned mesh node IDs for reordering
    //! \details The 'flatten' is used here as a concatenation of a data
    //!   structure that stores date categorized by chares owned on this PE. The
    //!   result of the flattening is thus a simpler data structure that is no
    //!   longer categorized by (or associated to) chares.
    void flatten() {
      // Optionally refine mesh if requested
      const auto ir = g_inputdeck.get< tag::selected, tag::initialamr >();
      if (ir == tk::ctr::InitialAMRType::UNIFORM) refine();
      // Make sure we are not fed garbage
      Assert( m_chinpoel.size() ==
                static_cast< std::size_t >( chareDistribution()[1] ),
              "Global mesh nodes ids associated to chares on PE " +
              std::to_string( CkMyPe() ) + " is incomplete" );
      // Collect chare IDs we own associated to old global mesh node IDs
      for (const auto& c : m_chinpoel)
        for (auto p : c.second)
          m_nodechares[p].push_back( c.first );
      // Make chare IDs (associated to old global mesh node IDs) unique
      for (auto& c : m_nodechares) tk::unique( c.second );
      // Collect chare IDs we own associated to edges
      for (const auto& c : m_chedgenodes)
        for (const auto& e : c.second)
          m_edgechares[ e.first ].push_back( c.first );
      // Make chare IDs (associated to edges) unique
      for (auto& c : m_edgechares) tk::unique( c.second );
      // Flatten node IDs of elements our chares operate on
      for (const auto& c : m_chinpoel)
        for (auto i : c.second)
          m_nodeset.insert( i );
      // Flatten edges of elements our chares operate on
      for (const auto& c : m_chedgenodes)
        for (const auto& e : c.second)
          m_edgeset.insert( e.first );
//       // send progress report to host
//       if ( g_inputdeck.get< tag::cmd, tag::feedback >() )
//         m_host.peflattened();
      // Signal host that we are ready for computing the communication map,
      // required for parallel distributed global mesh node reordering
      Group::contribute( m_cb.get< tag::flattened >() );
    }

    //! Receive lower bound of node IDs our PE operates on after reordering
    //! \param[in] low Lower bound of node IDs assigned to us
    void lower( std::size_t low ) {
      m_lower = low;
      lower_complete();
    }

    //! \brief Compute the variance of the communication cost of merging the
    //!   linear system
    //! \param[in] av Average of the communication cost
    //! \details Computing the standard deviation is done via computing and
    //!   summing up the variances on each PE and asynchronously reducing the
    //!   sum to our host.
    void stdCost( tk::real av ) {
      tk::real var = (m_cost-av)*(m_cost-av);
      Group::contribute( sizeof(tk::real), &var, CkReduction::sum_double,
                         m_cb.get< tag::stdcost >() );
    }

    //! \brief Start gathering global node IDs this PE will need to receive
    //!   (instead of assign) during reordering
    void gather() { Group::thisProxy.query( CkMyPe(), m_nodeset, m_edgeset ); }

    //! \brief Query our global node IDs and edges by other PEs so they know if
    //!   they are to receive IDs for those from during reordering
    //! \param[in] p Querying PE
    //! \param[in] nodes List of global mesh node IDs to query
    //! \param[in] edges List of edges to query
    //! \details Note that every PE calls this function in a broadcast fashion,
    //!   including our own. However, to compute the correct result, this would
    //!   only be necessary for PEs whose ID is higher than ours. However, the
    //!   broadcast (calling everyone) is more efficient. This also results in a
    //!   simpler logic, because every PE goes through this single call path.
    //!   The returned mask is simply a boolean array signaling if the node ID
    //!   is found (owned).
    void query( int p,
                const std::set< std::size_t >& nodes,
                const tk::UnsMesh::Edges& edges ) const
    {
      std::unordered_map< std::size_t, std::vector< int > > cn;
      for (auto j : nodes) {
        const auto it = m_nodeset.find( j );
        if (it != end(m_nodeset)) {
          const auto& c = tk::cref_find( m_nodechares, j );
          auto& chares = cn[j];
          chares.insert( end(chares), begin(c), end(c) );
        }
      }
      tk::UnsMesh::EdgeChares ce;
      for (auto j : edges) {
        const auto it = m_edgeset.find( j );
        if (it != end(m_edgeset)) {
          const auto& c = tk::cref_find( m_edgechares, j );
          auto& chares = ce[j];
          chares.insert( end(chares), begin(c), end(c) );
        }
      }
      Group::thisProxy[ p ].mask( CkMyPe(), cn, ce );
    }

    //! Receive mask of to-be-received global mesh node IDs
    //! \param[in] p The PE uniquely assigns the node IDs marked listed in ch
    //! \param[in] cn Vector containing the set of potentially multiple chare
    //!   IDs that we own (i.e., contribute to) for all of our node IDs.
    //! \param[in] ce Map associating a vector of chare IDs to edges (at which
    //!   we added nodes during initial mesh refinement)
    //! \details Note that every PE will call this function, since query() was
    //!   called in a broadcast fashion and query() answers to every PE once.
    //!   This is more efficient than calling only the PEs from which we would
    //!   have to receive results from. Thus the incoming results are only
    //!   interesting from PEs with lower IDs than ours.
    void mask( int p,
               const std::unordered_map< std::size_t, std::vector< int > >& cn,
               const tk::UnsMesh::EdgeChares& ce )
    {
      // Store the old global mesh node IDs associated to chare IDs bordering
      // the mesh chunk held by and associated to chare IDs we own
      for (const auto& h : cn) {
        const auto& chares = tk::ref_find( m_nodechares, h.first );
        for (auto c : chares) {           // surrounded chares
          auto& sch = m_msum[c];
          for (auto s : h.second)         // surrounding chares
            if (s != c) sch[ s ].insert( h.first );
        }
      }
      // Store the edges associated to chare IDs bordering the mesh chunk held
      // by and associated to chare IDs we own
      for (const auto& h : ce) {
        const auto& chares = tk::ref_find( m_edgechares, h.first );
        for (auto c : chares) {           // surrounded chares
          auto& sch = m_msumed[c];
          for (auto s : h.second)         // surrounding chares
            if (s != c) sch[ s ].insert( h.first );
        }
      }
      // Associate global mesh node IDs to lower PEs we will need to receive
      // from during node reordering. The choice of associated container is
      // std::map, which is ordered (vs. unordered, hash-map). This is required
      // by the following operation that makes the mesh node IDs unique in the
      // communication map. (We are called in an unordered fashion, so we need
      // to collect from all PEs and then we need to make the node IDs unique,
      // keeping only the lowest PEs a node ID is associated with.)
      if (p < CkMyPe()) {
        auto& id = m_ncomm[ p ];
        for (const auto& h : cn) id.insert( h.first );
        auto& ed = m_ecomm[ p ];
        for (const auto& h : ce) ed.insert( h.first );
      }
      if (++m_nquery == static_cast<std::size_t>(CkNumPes())) {
        // Make sure we have received all we need
        Assert( m_ncomm.size() == static_cast<std::size_t>(CkMyPe()),
                "Communication map size on PE " +
                std::to_string(CkMyPe()) + " must equal " +
                std::to_string(CkMyPe()) );
        Assert( m_ecomm.size() == static_cast<std::size_t>(CkMyPe()),
                "Communication map size on PE " +
                std::to_string(CkMyPe()) + " must equal " +
                std::to_string(CkMyPe()) );
        // Fill new hash-map, keeping only unique node IDs obtained from the
        // lowest possible PEs
        for (auto c=m_ncomm.cbegin(); c!=m_ncomm.cend(); ++c) {
          auto& n = m_ncommunication[ c->first ];
          for (auto j : c->second)
            if (std::none_of( m_ncomm.cbegin(), c,
                 [ j ]( const typename decltype(m_ncomm)::value_type& s )
                 { return s.second.find(j) != end(s.second); } )) {
              n.insert(j);
            }
          if (n.empty()) m_ncommunication.erase( c->first );
        }
        for (auto c=m_ecomm.cbegin(); c!=m_ecomm.cend(); ++c) {
          auto& e = m_ecommunication[ c->first ];
          for (const auto& j : c->second)
            if (std::none_of( m_ecomm.cbegin(), c,
                 [ &j ]( const typename decltype(m_ecomm)::value_type& s )
                 { return s.second.find(j) != end(s.second); } )) {
              e.insert(j);
            }
          if (e.empty()) m_ecommunication.erase( c->first );
        }
        // Free storage of temporary communication map used to receive global
        // mesh node IDs as it is no longer needed once the final communication
        // map is generated.
        tk::destroy( m_ncomm );
        // Free storage of temporary communication map used to receive global
        // mesh edge-node IDs as it is no longer needed once the final
        // communication map is generated.
        tk::destroy( m_ecomm );
        // Count up total number of nodes and (nodes associated to edges) we
        // will need receive during reordering
        std::size_t nrecv = 0, erecv = 0;
        for (const auto& u : m_ncommunication) nrecv += u.second.size();
        for (const auto& e : m_ecommunication) erecv += e.second.size();
//         // send progress report to host
//         if ( g_inputdeck.get< tag::cmd, tag::feedback >() )
//           m_host.pemask();
        // Compute number of mesh node IDs we will assign IDs to
        auto nuniq = m_nodeset.size() - nrecv + m_edgeset.size() - erecv;
        // Start computing PE offsets for node reordering
        Group::thisProxy.offset( CkMyPe(), nuniq );
      }
    }

  private:
    //! Charm++ callbacks associated to compile-time tags
    tk::tuple::tagged_tuple<
        tag::part,        CkCallback
      , tag::distributed, CkCallback
      , tag::flattened,   CkCallback
      , tag::load,        CkCallback
      , tag::avecost,     CkCallback
      , tag::stdcost,     CkCallback
    > m_cb;
    //! Host proxy
    CProxy_Transporter m_host;
    //! Worker proxy
    WorkerProxy m_worker;
    //! Linear system solver proxy
    tk::CProxy_Solver< WorkerProxy > m_solver;
    //! Number of fellow PEs to send elem IDs to
    std::size_t m_npe;
    //! Queue of requested node IDs from PEs
    std::vector< std::pair< int, std::unordered_set<std::size_t> > > m_reqNodes;
    //! Queue of requested edge-node IDs from PEs
    std::vector< std::pair< int, tk::UnsMesh::Edges > > m_reqEdges;
    //! \brief Starting global mesh node ID for node reordering on this PE
    //!   during mesh node reordering
    std::size_t m_start;
    //! \brief Counter for number of offsets
    //! \details This counts the to-be-received node IDs received while
    //!   computing global mesh node ID offsets for each PE rquired for node
    //!   reordering later
    std::size_t m_noffset;
    //! \brief Counter for number of masks of to-be-received global mesh node
    //!   IDs received
    //! \details This counts the to-be-received node ID masks received while
    //!   gathering the node IDs that need to be received (instead of uniquely
    //!   assigned) by each PE
    std::size_t m_nquery;
    //! Tetrtahedron element connectivity of our chunk of the mesh
    std::vector< std::size_t > m_tetinpoel;
    //! Global element IDs we read (our chunk of the mesh)
    std::vector< long > m_gelemid;
    //! Element centroid coordinates of our chunk of the mesh
    std::array< std::vector< tk::real >, 3 > m_centroid;
    //! Total number of chares across all PEs
    int m_nchare;
    //! Lower bound of node IDs our PE operates on after reordering
    std::size_t m_lower;
    //! Upper bound of node IDs our PE operates on after reordering
    std::size_t m_upper;
    //! \brief Temporary communication map used to receive global mesh node IDs
    //! \details This map, on each PE, associates the list of global mesh point
    //!   indices to fellow PE IDs from which we will receive new node IDs (as
    //!   in producing contiguous-row-id linear system contributions) during
    //!   reordering.
    std::map< int, std::unordered_set< std::size_t > > m_ncomm;
    //! \brief Temporary communication map used to receive global mesh edges
    //! \details This map, on each PE, associates the list of global mesh edges
    //!   indices to fellow PE IDs from which we will receive new nodes IDs (as
    //!   in producing contiguous-row-id linear system contributions) associated
    //!   to edges during reordering.
    std::map< int, tk::UnsMesh::Edges > m_ecomm;
    //! \brief Communication map used for distributed mesh node reordering
    //! \details This map, on each PE, associates the list of global mesh point
    //!   indices to fellow PE IDs from which we will receive new node IDs (as
    //!   in producing contiguous-row-id linear system contributions) during
    //!   reordering. Only data that will be received from PEs with a lower
    //!   index are stored.
    std::unordered_map< int, std::unordered_set<std::size_t> > m_ncommunication;
    //! \brief Communication map used for distributed mesh edge-node reordering
    //! \details This map, on each PE, associates the list of global mesh edges
    //!   to fellow PE IDs from which we will receive new node IDs (as in
    //!   producing contiguous-row-id linear system contributions)associated to
    //!   edges during reordering. Only data that will be received from PEs with
    //!   a lower index are stored.
    std::unordered_map< int, tk::UnsMesh::Edges > m_ecommunication;
    //! \brief Unique global node IDs chares on our PE will contribute to in a
    //!   linear system
    std::set< std::size_t > m_nodeset;
    //! \brief Unique global edges whose nodes chares on our PE will contribute
    //!   to in a linear system
    tk::UnsMesh::Edges m_edgeset;
    //! \brief Map associating new node IDs (as in producing contiguous-row-id
    //!   linear system contributions) as map-values to old node IDs (as in
    //!   file) as map-keys
    std::unordered_map< std::size_t, std::size_t > m_linnodes;
    //! \brief Map associating new node IDs (as in producing contiguous-row-id
    //!   linear system contributions) as map-values to edges given by two old
    //!   node IDs (as in file) as map-keys
    tk::UnsMesh::EdgeNodes m_linedges;
    //! Global mesh element connectivity associated to chares owned
    std::unordered_map< int, std::vector< std::size_t > > m_chinpoel;
    //! \brief Maps associating old node IDs to new node IDs (as in producing
    //!   contiguous-row-id linear system contributions) categorized by chares.
    //! \details Maps associating old node IDs (as in file) as map-values to new
    //!   node IDs (as in producing contiguous-row-id linear system
    //!   contributions) as map-keys, associated to chare IDs (outer keys).
    std::unordered_map< int,
      std::unordered_map< std::size_t, std::size_t > > m_chfilenodes;
    //! \brief Maps associating new node IDs (as in producing contiguous-row-id
    //!   linear system contributions) to edges (a pair of old node IDs) in
    //!   tk::UnsMesh::EdgeNodes maps, associated to and categorized by chares.
    //! \details Maps associating new node IDs (as in producing
    //!   contiguous-row-id linear system contributions) to edges (a pair of old
    //!   node IDs, as in file) associated to chare IDs (outer key) for only
    //!   the nodes newly added as a result of initial uniform refinement.
    //! \note Used for looking up boundary conditions, see, e.g., Carrier::bc()
    std::unordered_map< int, tk::UnsMesh::EdgeNodes > m_chedgenodes;
    //! Communication cost of linear system merging for our PE
    tk::real m_cost;
    //! \brief Map associating a set of chare IDs to old (as in file) global
    //!   mesh node IDs
    //! \details Note that a single global mesh ID can be associated to multiple
    //!   chare IDs as multiple chares can contribute to a single mesh node.
    std::unordered_map< std::size_t, std::vector< int > > m_nodechares;
    //! \brief Map associating a set of chare IDs to edges given by two old
    //!   global mesh node IDs (old as in file)
    //! \details Note that a single edge can be associated to multiple chare IDs
    //!   as multiple chares can contribute to a single edge.
    tk::UnsMesh::EdgeChares m_edgechares;
    //! \brief Global mesh node IDs associated to chare IDs bordering the mesh
    //!   chunk held by (and associated to) chare IDs this PE owns
    //! \details msum: (M)esh chunks (S)urrounding (M)esh chunks storing mesh
    //!   nodes. Outer map-key: chare IDs this PE owns whose neighbors are
    //!   stored, inner map-key: chare IDs of those chares that hold mesh chunks
    //!   surrounding the outer-key chare's mesh, map-values: global new
    //!   (reordered, as in producing contiguous-row-id linear system
    //!   contributions) mesh node IDs along the border of chares (at which the
    //!   chares will need to communicate) during time stepping.
    std::unordered_map< int,
      std::unordered_map< int, std::unordered_set< std::size_t > > > m_msum;
    //! \brief Mesh edges given by two global mesh node IDs associated to chare
    //!   IDs bordering the mesh chunk held by (and associated to) chare IDs
    //!   this PE owns
    //! \details msum: (M)esh chunks (S)urrounding (M)esh chunks storing mesh
    //!   nodes. Outer map-key: chare IDs this PE owns whose neighbors are
    //!   stored, inner map-key: chare IDs of those chares that hold mesh chunks
    //!   surrounding the outer-key chare's mesh, map-values: mesh edges given
    //!   by two global new (reordered, as in producing contiguous-row-id linear
    //!   system contributions) mesh node IDs along the border of chares (at
    //!   which the chares will need to communicate) during time stepping.
    std::unordered_map< int,
      std::unordered_map< int, tk::UnsMesh::Edges > > m_msumed;

    //! Read our contiguously-numbered chunk of the mesh graph from file
    //! \param[in] er ExodusII mesh reader
    void readGraph( tk::ExodusIIMeshReader& er ) {
      // Get number of mesh points and number of tetrahedron elements in file
      er.readElemBlockIDs();
      auto nel = er.nelem( tk::ExoElemType::TET );
      // Read our contiguously-numbered chunk of tetrahedron element
      // connectivity from file and also generate and store the list of global
      // element indices for our chunk of the mesh
      auto npes = static_cast< std::size_t >( CkNumPes() );
      auto mype = static_cast< std::size_t >( CkMyPe() );
      auto chunk = nel / npes;
      auto from = mype * chunk;
      auto till = from + chunk;
      if (mype == npes-1) till += nel % npes;
      er.readElements( {{from, till-1}}, tk::ExoElemType::TET, m_tetinpoel );
      m_gelemid.resize( till-from );
      std::iota( begin(m_gelemid), end(m_gelemid), from );
//       // send progress report to host
//       if ( g_inputdeck.get< tag::cmd, tag::feedback >() )
//         m_host.pegraph();
      uint64_t nelem = m_gelemid.size();
      Group::contribute( sizeof(uint64_t), &nelem, CkReduction::sum_int,
                         m_cb.get< tag::load >() );
    }

    // Compute element centroid coordinates
    //! \param[in] er ExodusII mesh reader
    void computeCentroids( tk::ExodusIIMeshReader& er ) {
      // Construct unique global mesh point indices of our chunk
      auto gid = m_tetinpoel;
      tk::unique( gid );
      // Read node coordinates of our chunk of the mesh elements from file
      auto ext = tk::extents( gid );
      auto coord = er.readNodes( ext );
      const auto& x = std::get< 0 >( coord );
      const auto& y = std::get< 1 >( coord );
      const auto& z = std::get< 2 >( coord );
      // Make room for element centroid coordinates
      auto& cx = m_centroid[0];
      auto& cy = m_centroid[1];
      auto& cz = m_centroid[2];
      auto num = m_tetinpoel.size()/4;
      cx.resize( num );
      cy.resize( num );
      cz.resize( num );
      // Compute element centroids for our chunk of the mesh elements
      for (std::size_t e=0; e<num; ++e) {
        auto A = m_tetinpoel[e*4+0] - ext[0];
        auto B = m_tetinpoel[e*4+1] - ext[0];
        auto C = m_tetinpoel[e*4+2] - ext[0];
        auto D = m_tetinpoel[e*4+3] - ext[0];
        cx[e] = (x[A] + x[B] + x[C] + x[D]) / 4.0;
        cy[e] = (y[A] + y[B] + y[C] + y[D]) / 4.0;
        cz[e] = (z[A] + z[B] + z[C] + z[D]) / 4.0;
      }
      Group::contribute( m_cb.get< tag::part >() );
    }

    //! Construct global mesh node ids for each chare
    //! \param[in] che Chares of elements: array of chare ownership IDs mapping
    //!   graph elements to Charm++ chares. Size: number of elements in the
    //!   chunk of the mesh graph on this PE.
    //! \return Vector of global mesh node ids connecting elements owned by each
    //!   chare on this PE
    //! \note The chare IDs, as keys in the map constructed here, are simply the
    //!   chare IDs returned by the partitioner assigning mesh elements to these
    //!   chares. It does not mean that these chare IDs are owned on this PE.
    std::unordered_map< int, std::vector< std::size_t > >
    chareNodes( const std::vector< std::size_t >& che ) const
    {
      Assert( che.size() == m_gelemid.size(), "The size of the global element "
              "index and the chare element arrays must equal" );
      Assert( che.size() == m_tetinpoel.size()/4, "The size of the mesh "
              "connectivity / 4 and the chare element arrays must equal" );
      // Categorize global mesh node ids of elements by chares
      std::unordered_map< int, std::vector< std::size_t > > nodes;
      for (std::size_t e=0; e<che.size(); ++e) {
        auto& c = nodes[ static_cast<int>(che[e]) ];
        for (std::size_t n=0; n<4; ++n) c.push_back( m_tetinpoel[e*4+n] );
      }
      // Make sure all PEs have chares assigned
      Assert( !nodes.empty(), "No nodes have been assigned to chares on PE " +
              std::to_string(CkMyPe()) );
      // This check should always be done, hence ErrChk and not Assert, as it
      // can result from particular pathological combinations of (1) too large
      // degree of virtualization, (2) too many PEs, and/or (3) too small of a
      // mesh and not due to programmer error.
      for(const auto& c : nodes)
        ErrChk( !c.second.empty(),
                "Overdecomposition of the mesh is too large compared to the "
                "number of work units computed based on the degree of "
                "virtualization desired. As a result, there would be at least "
                "one work unit with no mesh elements to work on, i.e., nothing "
                "to do. Solution 1: decrease the virtualization to a lower "
                "value using the command-line argument '-u'. Solution 2: "
                "decrease the number processing elements (PEs) using the "
                "charmrun command-line argument '+pN' where N is the number of "
                "PEs, which implicitly increases the size (and thus decreases "
                "the number) of work units.)" );
      return nodes;
    }

    //! Distribute global mesh node IDs to their owner PEs
    //! \param[in] n Global mesh node IDs connecting elements associated to
    //!   chare IDs on this PE resulting from partitioning the mesh elements.
    //!   Note that this data is moved in.
    void distribute( std::unordered_map< int, std::vector< std::size_t > >&& n )
    {
      auto dist = chareDistribution();
      for (int c=0; c<dist[1]; ++c) {
        auto chid = CkMyPe() * dist[0] + c;   // compute owned chare ID
        const auto it = n.find( chid );       // attempt to find its nodes
        if (it != end(n)) {                   // if found
          m_chinpoel.insert( *it );           // move over owned key-value pairs
          n.erase( it );                      // remove chare ID and nodes
        }
        Assert( n.find(chid) == end(n), "Not all owned node IDs stored" );
      }
      // Construct export map associating those map entries (mesh node indices
      // associated to chare IDs) owned by chares we do not own. Outer key: PE
      // to export to, inner key: chare ID, value: vector of global node IDs
      std::unordered_map< int,
        std::unordered_map< int, std::vector< std::size_t > > > exp;
      for (auto&& c : n) exp[ pe(c.first) ].insert( std::move(c) );
      // Export chare IDs and node IDs we do not own to fellow PEs
      m_npe = exp.size();
      for (const auto& p : exp)
        Group::thisProxy[ p.first ].add( CkMyPe(), p.second );
//       // send progress report to host
//       if ( g_inputdeck.get< tag::cmd, tag::feedback >() )
//         m_host.pedistributed();
      if (m_npe == 0) Group::contribute( m_cb.get< tag::distributed >() );
    }

    //! Compute chare distribution
    //! \return Chunksize, i.e., number of chares per all PEs except the last
    //!   one, and the number of chares for my PE
    //! \details Chare ids are distributed to PEs in a linear continguous order
    //!   with the last PE taking the remainder if the number of PEs is not
    //!   divisible by the number chares. For example, if nchare=7 and npe=3,
    //!   the chare distribution is PE0: 0 1, PE1: 2 3, and PE2: 4 5 6. As a
    //!   result of this distribution, all PEs will have their chare-categorized
    //!   element connectivity filled with the global mesh node IDs associated
    //!   to the Charm++ chare IDs each PE owns.
    std::array< int, 2 > chareDistribution() const {
      auto chunksize = m_nchare / CkNumPes();
      auto mynchare = chunksize;
      if (CkMyPe() == CkNumPes()-1) mynchare += m_nchare % CkNumPes();
      return {{ chunksize, mynchare }};
    }

    //! Reorder global mesh node IDs
    void reorder() {
      // Activate SDAG waits for having requests arrive from other PEs for some
      // of our node IDs; and for computing/receiving lower and upper bounds of
      // global node IDs our PE operates on after reordering
      wait4prep();
      wait4bounds();
      wait4reorder();
      // In serial signal to the runtime system that we have participated in
      // reordering. This is required here because this is only triggered if
      // communication is required during mesh node reordering. See also
      // particioner.ci.
      if (CkNumPes() == 1) participated_complete();
      // Send out request for new global node IDs for nodes we do not reorder
      for (const auto& c : m_ncommunication)
        Group::thisProxy[ c.first ].request( CkMyPe(), c.second );
      // Send out request for new global node IDs for edges we do not reorder
      for (const auto& e : m_ecommunication)
        Group::thisProxy[ e.first ].request( CkMyPe(), e.second );
      // Lambda to decide if node ID is being assigned a new ID by us
      auto ownnode = [ this ]( std::size_t p ) {
        using Set = typename std::remove_reference<
                      decltype(m_ncommunication) >::type::value_type;
        return !std::any_of( m_ncommunication.cbegin(), m_ncommunication.cend(),
                             [&](const Set& s)
                             { return s.second.find(p) != s.second.cend(); } );
      };
      // Lambda to decide if edge-node ID is being assigned a new ID by us
      auto ownedge = [ this ]( const tk::UnsMesh::Edge& e ) {
        using Set = typename std::remove_reference<
                      decltype(m_ecommunication) >::type::value_type;
        return !std::any_of( m_ecommunication.cbegin(), m_ecommunication.cend(),
                             [&](const Set& s)
                             { return s.second.find(e) != s.second.cend(); } );
      };
      // Reorder our chunk of the mesh node IDs by looping through all of our
      // node IDs (resulting from reading our chunk of the mesh cells). We test
      // if we are to assign a new ID to a node ID, and if so, we assign new ID,
      // i.e., reorder, by constructing a map associating new to old IDs. We
      // also count up the reordered nodes, which also serves as the new node
      // id.
      for (auto p : m_nodeset)
        if (ownnode(p)) m_linnodes[ p ] = m_start++;
      // Reorder our chunk of the mesh edges by looping through all of our edges
      // (resulting from initial uniform refinement of our chunk of the mesh
      // cells). We test if we are to assign a new ID to an edge, and if so, we
      // assign new ID, i.e., reorder, by constructing a map associating new
      // node IDs to edges. We also count up the reordered edge-nodes, which
      // also serves as the new node id.
      for (const auto& e : m_edgeset)
        if (ownedge(e)) m_linedges[ e ] = m_start++;
      // Trigger SDAG wait indicating that reordering own node IDs are complete
      reorderowned_complete();
      // If all our nodes have new IDs assigned, signal that to the runtime
      if (m_linnodes.size() == m_nodeset.size()) nodesreorder_complete();
      // If all our edges have new IDs assigned, signal that to the runtime
      if (m_linedges.size() == m_edgeset.size()) edgesreorder_complete();
    }

    //! Return processing element for chare id
    //! \param[in] id Chare id
    //! \return PE that creates the chare
    //! \details This is computed based on a simple contiguous linear
    //!   distribution of chare ids to PEs.
    int pe( int id ) const {
      Assert( m_nchare > 0, "Number of chares must be a positive number" );
      auto p = id / (m_nchare / CkNumPes());
      if (p >= CkNumPes()) p = CkNumPes()-1;
      Assert( p < CkNumPes(), "Assigning to nonexistent PE" );
      return p;
    }

    //! Associate new node IDs to old ones and return them to the requestor(s)
    void prepare() {
      // Signal to the runtime system that we have participated in reordering
      participated_complete();
      // Find and return new node IDs to sender
      for (const auto& r : m_reqNodes) {
        std::unordered_map< std::size_t, std::size_t > n;
        for (auto p : r.second) n[ p ] = tk::cref_find( m_linnodes, p );
        Group::thisProxy[ r.first ].neworder( n );
        tk::destroy( n );
      }
      tk::destroy( m_reqNodes ); // Clear queue of requests just fulfilled
      // Find and return new node IDs associated to edges to sender
      for (const auto& r : m_reqEdges) {
        tk::UnsMesh::EdgeNodes n;
        for (const auto& e : r.second) n[ e ] = tk::cref_find( m_linedges, e );
        Group::thisProxy[ r.first ].neworder( n );
        tk::destroy( n );
      }
      tk::destroy( m_reqEdges ); // Clear queue of requests just fulfilled
      wait4prep();      // Re-enable SDAG wait for preparing new node requests
      // Re-enable trigger signaling that reordering of owned node IDs are
      // complete right away
      reorderowned_complete();
    }

    void generate_compact_inpoel()
    {
        // Concatenate mesh connectivities of our chares
        tk::destroy(m_tetinpoel);

        std::size_t nn = 0;

        // M_chinpoel contains an array of size chars.
        // Each slot in the array contains "connectivity"
        // This is a vector of size_t. This is tets?

        // This loop counts up the total number of elements in all lsots of
        // m_chinpoel
        for (const auto &c : m_chinpoel)
        {
            nn += c.second.size();
        }

        // Resize global array to avoid resize later
        m_tetinpoel.resize(nn);

        // Flatten out m_chinpoel into m_tetinpoel
        size_t n = 0;
        for (const auto &c : m_chinpoel) {
            for (auto i : c.second) {
                m_tetinpoel[n++] = i;
            }
        }

    }

    //! Uniformly refine our mesh replacing each tetrahedron with 8 new ones
    void refine()
    {

        generate_compact_inpoel();

        // Create AMR object
        AMR::mesh_adapter_t* mesh_adapter = new AMR::mesh_adapter_t();

        // Generate unique edges (nodes connected to nodes)?

        // shift to zero-based node IDs
        // Find min/max to shift
        auto minmax = std::minmax_element(begin(m_tetinpoel), end(m_tetinpoel));

        std::array<std::size_t, 2> ext{{*minmax.first, *minmax.second}};
        auto nnode = ext[1] - ext[0] + 1;

        mesh_adapter->init(m_tetinpoel, nnode);

        // Do uniform refinement
        mesh_adapter->uniform_refinement();

        // TODO: Do we need to replicate the shift?

        /*
        // Perform shift
        for (auto &i : m_tetinpoel) {
        i -= ext[0];
        }

        // TODO: Is there a reason we want to not enforce 0 based no ids
        // always? (i.e not put them back?)

        // Generate elements surrounding points..? If we have tets, why do we
        // need to do this? Can't we just generate edges?
        auto esup = tk::genEsup(m_tetinpoel, 4);

        for (auto &i : m_tetinpoel) i += ext[0];  // shift back node IDs

        std::unordered_map<std::size_t, std::unordered_set<std::size_t> > star;

        for (std::size_t j = 0; j < nnode; ++j) {
        for (std::size_t i = esup.second[j] + 1; i <= esup.second[j + 1]; ++i) {
        for (std::size_t n = 0; n < 4; ++n) {
        auto p = ext[0] + j;
        auto q = m_tetinpoel[esup.first[i] * 4 + n];
        if (p < q) star[p].insert(q);
        if (p > q) star[q].insert(p);
        }
        }
        }

        // Starting node ID (on all PEs) while assigning new edge-nodes
        nnode = tk::ExodusIIMeshReader(g_inputdeck.get<tag::cmd, tag::io,
        tag::input>()).readHeader();

        // Add new edge-nodes
        tk::UnsMesh::EdgeNodes edgenodes;
        for (const auto &s : star) {
        for (auto q : s.second) {
        edgenodes[ {{ s.first, q }} ] = nnode++;
        }
        }
        */

        tk::destroy(m_tetinpoel);

        // Generate maps associating new node IDs (as in producing
        // contiguous-row-id linear system contributions)to edges (a pair of old
        // node IDs) in tk::UnsMesh::EdgeNodes maps, associated to and categorized
        // by chares. Note that the new edge-node IDs assigned here will be
        // overwritten with globally unique node IDs after reordering.
        for (const auto& conn : m_chinpoel) {
            auto& en = m_chedgenodes[ conn.first ];
            for (std::size_t e=0; e<conn.second.size()/4; ++e) {
                const auto A = conn.second[e*4+0];
                const auto B = conn.second[e*4+1];
                const auto C = conn.second[e*4+2];
                const auto D = conn.second[e*4+3];
                // Look up the added node IDs based on old ids {A,B}
                // (vector)

                /*
                   const auto AB = tk::cref_find( edgenodes, {{ A,B }} );
                   const auto AC = tk::cref_find( edgenodes, {{ A,C }} );
                   const auto AD = tk::cref_find( edgenodes, {{ A,D }} );
                   const auto BC = tk::cref_find( edgenodes, {{ B,C }} );
                   const auto BD = tk::cref_find( edgenodes, {{ B,D }} );
                   const auto CD = tk::cref_find( edgenodes, {{ C,D }} );
                   */

                // TODO: We should likely check the return values here
                const int AB = mesh_adapter->node_connectivity.find(A, B);
                const int AC = mesh_adapter->node_connectivity.find(A, C);
                const int AD = mesh_adapter->node_connectivity.find(A, D);
                const int BC = mesh_adapter->node_connectivity.find(B, C);
                const int BD = mesh_adapter->node_connectivity.find(B, D);
                const int CD = mesh_adapter->node_connectivity.find(C, D);

                en[ {{A,B}} ] = static_cast<size_t>(AB);
                en[ {{A,C}} ] = static_cast<size_t>(AC);
                en[ {{A,D}} ] = static_cast<size_t>(AD);
                en[ {{B,C}} ] = static_cast<size_t>(BC);
                en[ {{B,D}} ] = static_cast<size_t>(BD);
                en[ {{C,D}} ] = static_cast<size_t>(CD);
            }
        }

        generate_compact_inpoel();

        delete mesh_adapter;

        // TODO: This only needs to have set en? Which is m_chedgenodes.
    }

    //! Compute final result of reordering
    //! \details This member function is called when both those node IDs that we
    //!   assign a new ordering to as well as those assigned new IDs by other
    //!   PEs have been reordered (and we contribute to) and we are ready (on
    //!   this PE) to compute our final result of the reordering.
    void reordered() {
      // Free memory used by communication maps used to store nodes and
      // edge-nodes and associated PEs during reordering.
      tk::destroy( m_ncommunication );
      tk::destroy( m_ecommunication );
      // Free memory used by maps associating a list of chare IDs to old (as in
      // file) global mesh node IDs and to edges as no longer needed.
      tk::destroy( m_nodechares );
      tk::destroy( m_edgechares );
      // Construct maps associating old node IDs (as in file) to new node IDs
      // (as in producing contiguous-row-id linear system contributions)
      // associated to chare IDs (outer key).
      for (const auto& c : m_chinpoel) {
        auto& nodes = m_chfilenodes[ c.first ];
        for (auto p : c.second) {
          auto n = m_linnodes.find(p);
          if (n != end(m_linnodes)) nodes[ n->second ] = p;
        }
      }
      // Update node IDs of edges, i.e., the map values
      for (auto& c : m_chedgenodes)
        for (auto& e : c.second)
           e.second = tk::ref_find( m_linedges, e.first );

      if (!m_chedgenodes.empty()) {

        // Update chare-categorized element connectivities with new nodes and
        // newly added edge-nodes during initial unifor mesh refinement
        decltype(m_chinpoel) refinpoel;
        for (const auto& chi : m_chinpoel) {
          auto& ri = refinpoel[ chi.first ];
          const auto& edgenodes = tk::cref_find( m_chedgenodes, chi.first );
          for (std::size_t e=0; e<chi.second.size()/4; ++e) {
            auto A = chi.second[e*4+0];
            auto B = chi.second[e*4+1];
            auto C = chi.second[e*4+2];
            auto D = chi.second[e*4+3];
            const auto nA = tk::cref_find( m_linnodes, A );
            const auto nB = tk::cref_find( m_linnodes, B );
            const auto nC = tk::cref_find( m_linnodes, C );
            const auto nD = tk::cref_find( m_linnodes, D );
            const auto AB = tk::cref_find( edgenodes, {{ A,B }} );
            const auto AC = tk::cref_find( edgenodes, {{ A,C }} );
            const auto AD = tk::cref_find( edgenodes, {{ A,D }} );
            const auto BC = tk::cref_find( edgenodes, {{ B,C }} );
            const auto BD = tk::cref_find( edgenodes, {{ B,D }} );
            const auto CD = tk::cref_find( edgenodes, {{ C,D }} );
            // update connectivity of our mesh chunk
            std::vector< std::size_t > newelems{{ nA, AB, AC, AD,
                                                  nB, BC, AB, BD,
                                                  nC, AC, BC, CD,
                                                  nD, AD, CD, BD,
                                                  BC, CD, AC, BD,
                                                  AB, BD, AC, AD,
                                                  AB, BC, AC, BD,
                                                  AC, BD, CD, AD }};
            ri.insert( end(ri), begin(newelems), end(newelems) );
          }
        }
        m_chinpoel = std::move( refinpoel );

        // Update chare-categorized mesh nodes surrounding our mesh chunk with
        // the reordered node IDs
        decltype(m_msum) newmsum;
        for (const auto& c : m_msum) {
          auto& m = newmsum[ c.first ];
          for (const auto& e : c.second) {
            auto& s = m[ e.first ];
            for (auto n : e.second)
              s.insert( tk::cref_find( m_linnodes, n ) );
          }
        }
        m_msum = std::move( newmsum );

        // Add newly added edge-nodes to chare-categorized mesh nodes
        // surrounding our mesh chunk
        for (const auto& c : m_msumed) {
          auto& sur = tk::cref_find( m_msum, c.first );
          const auto& edgenodes = tk::cref_find( m_chedgenodes, c.first );
          for (const auto& e : c.second) {
            auto& s = tk::ref_find( sur, e.first );
            for (const auto& ed : e.second)
              s.insert( tk::cref_find( edgenodes, ed ) );
          }
        }
        tk::destroy( m_msumed );

        // Update node IDs of edges, i.e., the map keys
        for (auto& c : m_chedgenodes) {
          tk::UnsMesh::EdgeNodes edgenodes;
          for (auto& e : c.second)
             edgenodes[ {{ tk::ref_find(m_linnodes,e.first[0]),
                           tk::ref_find(m_linnodes,e.first[1]) }} ] = e.second;
          c.second = std::move( edgenodes );
        }

      } else {

        // Update chare-categorized element connectivities with the reordered
        // node IDs
        for (auto& c : m_chinpoel)
          for (auto& p : c.second) {
            auto n = m_linnodes.find(p);
            if (n != end(m_linnodes)) p = n->second;
          }

        // Update chare-categorized mesh nodes surrounding our mesh chunk with
        // the reordered node IDs
        for (auto& c : m_msum)
          for (auto& s : c.second) {
            decltype(s.second) n;
            for (auto p : s.second) {
              auto it = m_linnodes.find(p);
              if (it != end(m_linnodes)) n.insert( it->second );
            }
            s.second = std::move( n );
          }

      }

      // Update unique global node IDs chares on our PE will contribute to with
      // the reordered node IDs
      tk::destroy( m_nodeset );
      for (const auto& c : m_chinpoel)
        for (auto i : c.second)
          m_nodeset.insert( i );

//       // send progress report to host
//       if ( g_inputdeck.get< tag::cmd, tag::feedback >() )
//         m_host.pereordered();
      // Compute lower and upper bounds of reordered node IDs our PE operates on
      bounds();
    }

    // Compute lower and upper bounds of reordered node IDs our PE operates on
    // \details This function computes the global row IDs at which the linear
    //   system will have a PE boundary. We simply find the largest node ID
    //   assigned on each PE by the reordering and use that as the upper global
    //   row index. Note that while this rarely results in equal number of rows
    //   assigned to PEs, potentially resulting in some load-imbalance, it
    //   yields a pretty good division reducing communication costs during the
    //   assembly of the linear system, which is more important than a slight
    //   (FLOP) load imbalance. Since the upper index for PE 1 is the same as
    //   the lower index for PE 2, etc., we find the upper indices and then the
    //   lower indices for all PEs are communicated.
    void bounds() {
      m_upper = 0;
      using P1 = std::pair< const std::size_t, std::size_t >;
      for (const auto& c : m_chfilenodes) {
        auto x = std::max_element( begin(c.second), end(c.second),
                 [](const P1& a, const P1& b){ return a.first < b.first; } );
        if (x->first > m_upper) m_upper = x->first;
      }
      using P2 = std::pair< const tk::UnsMesh::Edge, std::size_t >;
      for (const auto& c : m_chedgenodes) {
        auto x = std::max_element( begin(c.second), end(c.second),
                 [](const P2& a, const P2& b){ return a.second < b.second; } );
        if (x->second > m_upper) m_upper = x->second;
      }

      // The bounds are the dividers (global mesh point indices) at which the
      // linear system assembly is divided among PEs. However, Hypre and thus
      // Solver expect exclusive upper indices, so we increase the last one by
      // one here. Note that the cost calculation, Partitioner::cost() also
      // expects exclusive upper indices.
      if (CkMyPe() == CkNumPes()-1) ++m_upper;
      // Tell the runtime system that the upper bound has been computed
      upper_complete();
      // Set lower index for PE 0 as 0
      if (CkMyPe() == 0) lower(0);
      // All PEs except the last one send their upper indices as the lower index
      // for PE+1
      if (CkMyPe() < CkNumPes()-1)
        Group::thisProxy[ CkMyPe()+1 ].lower( m_upper );
    }

    //! \brief Create chare array elements on this PE and assign the global mesh
    //!   element IDs they will operate on
    //! \details We create chare array elements by calling the insert() member
    //!   function, which allows specifying the PE on which the array element is
    //!   created and we send each chare array element the global mesh element
    //!   connectivity, i.e., node IDs, it contributes to and the old->new node
    //!   ID map.
    void create() {
//       // send progress report to host
//       if ( g_inputdeck.get< tag::cmd, tag::feedback >() )
//         m_host.pebounds();
      // Initiate asynchronous reduction across all Partitioner objects
      // computing the average communication cost of merging the linear system
      m_cost = cost( m_lower, m_upper );
      Group::contribute( sizeof(tk::real), &m_cost, CkReduction::sum_double,
                         m_cb.get< tag::avecost >() );
      // Create worker chare array elements
      createWorkers( chareDistribution() );
      // Broadcast our bounds of global node IDs to all linear system solvers
      m_solver.bounds( CkMyPe(), m_lower, m_upper );
    }

    //! Create chare array elements on this PE
    //! \param[in] dist Pair of 'chunksize', i.e., number of chares per all PEs
    //!   except the last one, and 'mynchare', i.e., the number of chares for my
    //!   PE. See also chareDistribution().
    void createWorkers( std::array< int, 2 >&& dist ) {
      for (int c=0; c<dist[1]; ++c) {
        // Compute chare ID
        auto cid = CkMyPe() * dist[0] + c;
        // Guard those searches that operate on empty containers in serial
        typename decltype(m_msum)::mapped_type msum;
        if (!m_msum.empty()) msum = tk::cref_find( m_msum, cid );
        typename decltype(m_chedgenodes)::mapped_type edno;
        if (!m_chedgenodes.empty()) edno = tk::cref_find( m_chedgenodes, cid );
        // Create worker array element
        m_worker[ cid ].insert( m_host,
                                m_solver,
                                tk::cref_find( m_chinpoel, cid ),
                                msum,
                                tk::cref_find( m_chfilenodes, cid ),
                                edno,
                                m_nchare,
                                CkMyPe() );
      }
      m_worker.doneInserting();
      // Free storage for unique global mesh nodes chares on our PE will
      // contribute to in a linear system as no longer needed.
      tk::destroy( m_nodeset );
      // Free storage for unique global mesh edges whose nodes chares on our
      // PE will contribute to in a linear system as no longer needed.
      tk::destroy( m_edgeset );
      // Free storage of global mesh node ids associated to chares owned as it
      // is no longer needed after creating the workers.
      tk::destroy( m_chinpoel );
      // Free maps associating old node IDs to new node IDs categorized by
      // chares as it is no longer needed after creating the workers.
      tk::destroy( m_chfilenodes );
      // Free map storing new node IDs associated to edges categorized by chares
      // owned as no linger needed after creating workers.
      tk::destroy( m_chedgenodes );
      // Free storage of map associating a set of chare IDs to old global mesh
      // node IDs as it is no longer needed after creating the workers.
      tk::destroy( m_nodechares );
      // Free storage of global mesh node IDs associated to chare IDs bordering
      // the mesh chunk held by and associated to chare IDs we own as it is no
      // longer needed after creating the workers.
      tk::destroy( m_msum );
    }

    //! Compute communication cost of linear system merging for our PE
    //! \param[in] l Lower global row ID of linear system this PE works on
    //! \param[in] u Upper global row ID of linear system this PE works on
    //! \return Communication cost of merging the linear system for our PE
    //! \details The cost is a real number between 0 and 1, defined as the
    //!   number of mesh points we do not own, i.e., need to send to some other
    //!   PE, divided by the total number of points we contribute to. The lower
    //!   the better.
    tk::real cost( std::size_t l, std::size_t u ) {
      std::size_t ownpts = 0, compts = 0;
      for (auto p : m_nodeset) if (p >= l && p < u) ++ownpts; else ++compts;
      // Free storage of unique global node IDs chares on our PE will contribute
      // to in a linear system as it is no longer needed after computing the
      // communication cost.
      tk::destroy( m_nodeset );
      return static_cast<tk::real>(compts) /
             static_cast<tk::real>(ownpts + compts);
    }

    #if defined(__clang__)
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdocumentation"
    #endif
    /** @name Host signal calls
      * \brief These functions signal back to the host via a global reduction
      *   originating from each PE branch
      * \details Singal calls contribute to a reduction on all branches (PEs)
      *   of Solver to the host, e.g., inciter::CProxy_Transporter, given by the
      *   template argument HostProxy. The signal functions are overloads on the
      *   specialization, e.g., inciter::CProxy_Transporter, of the Solver
      *   template. They create Charm++ reduction targets via creating a
      *   callback that invokes the typed reduction client, where host is the
      *   proxy on which the reduction target method, given by the string
      *   followed by "redn_wrapper_", e.g., rowcomplete(), is called upon
      *   completion of the reduction.
      *
      *   Note that we do not use Charm++'s CkReductionTarget macro here,
      *   but instead explicitly generate the code that that macro would
      *   generate. To explain why, here is Charm++'s CkReductionTarget macro's
      *   definition, given in ckreduction.h:
      *   \code{.cpp}
      *      #define CkReductionTarget(me, method) \
      *        CkIndex_##me::redn_wrapper_##method(NULL)
      *   \endcode
      *   This macro takes arguments 'me' (a class name) and 'method' a member
      *   function of class 'me' and generates the call
      *   'CkIndex_<class>::redn_wrapper_<method>(NULL)'. With the overloads the
      *   signal2* functions generate, we do the above macro's job for
      *   Solver specialized by HostProxy, hard-coded here, as well its
      *   reduction target. This is required since
      *    * Charm++'s CkReductionTarget macro's preprocessing happens earlier
      *    than type resolution and the string of the template argument would be
      *    substituted instead of the type specialized (which is not what we
      *    want here), and
      *    * the template argument class, e.g, CProxy_Transporter, is in a
      *    namespace different than that of Solver. When a new class is used to
      *    specialize Solver, the compiler will alert that a new overload needs
      *    to be defined.
      *
      * \note This simplifies client-code, e.g., inciter::Transporter, which now
      *   requires no explicit book-keeping with counters, etc. Also a reduction
      *   (instead of a direct call to the host) better utilizes the
      *   communication network as computational nodes can send their aggregated
      *   contribution to other nodes on a network instead of all chares sending
      *   their (smaller) contributions to the same host, (hopefully)
      *   implemented using a tree among the PEs.
      * \see http://charm.cs.illinois.edu/manuals/html/charm++/manual.html,
      *   Sections "Processor-Aware Chare Collections" and "Chare Arrays".
      * */
    ///@{
//     //! Compute average communication cost of merging the linear system
//     //! \param[in] host Host to signal to
//     //! \details This is done via a Charm++ typed reduction, adding up the cost
//     //!   across all PEs and reducing the result to our host chare.
//     void signal2host_avecost( const CProxy_Transporter& host ) {
//       m_cost = cost( m_lower, m_upper );
//       Group::contribute( sizeof(tk::real), &m_cost, CkReduction::sum_double,
//                          CkCallback( CkReductionTarget(Transporter,aveCost),
//                          host ));
//     }
//     //! \brief Compute standard deviation of the communication cost of merging
//     //!   the linear system
//     //! \param[in] host Host to signal to
//     //! \param[in] var Square of the communication cost minus the average for
//     //!   our PE.
//     //! \details This is done via a Charm++ typed reduction, adding up the
//     //!   squares of the communication cost minus the average across all PEs and
//     //!   reducing the result to our host chare.
//     void signal2host_stdcost( const CProxy_Transporter& host, tk::real var ) {
//       Group::contribute( sizeof(tk::real), &var, CkReduction::sum_double,
//                          CkCallback( CkReductionTarget(Transporter,stdCost),
//                          host ));
//     }
    ///@}
    #if defined(__clang__)
      #pragma clang diagnostic pop
    #endif
};

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

} // inciter::

#define CK_TEMPLATES_ONLY
#include "NoWarning/partitioner.def.h"
#undef CK_TEMPLATES_ONLY

#endif // Partitioner_h
