// *****************************************************************************
/*!
  \file      src/Inciter/DistFCT.C
  \copyright 2012-2015, J. Bakosi, 2016-2018, Los Alamos National Security, LLC.
  \brief     Charm++ module interface for distributed flux-corrected transport
  \details   Charm++ module interface file for asynchronous distributed
    flux-corrected transport (FCT).
  \see       DistFCT.[Ch] and FluxCorrector.[Ch] for more info.
*/
// *****************************************************************************

#include <string>
#include <cmath>
#include <array>
#include <set>
#include <algorithm>

#include "QuinoaConfig.h"
#include "ContainerUtil.h"
#include "DistFCT.h"
#include "Variant.h"

namespace inciter {

extern ctr::InputDeck g_inputdeck;

} // inciter::

using inciter::DistFCT;

DistFCT::DistFCT( const CProxy_Transporter& host,
                  int nchare,
                  std::size_t nu,
                  std::size_t np,
                  const std::unordered_map< int,
                    std::vector< std::size_t > >& msum,
                  const std::unordered_map< std::size_t, std::size_t >& bid,
                  const std::unordered_map< std::size_t, std::size_t >& lid,
                  const std::vector< std::size_t >& inpoel ) :
  m_host( host ),
  m_nhsol( 0 ),
  m_nlsol( 0 ),
  m_naec( 0 ),
  m_nalw( 0 ),
  m_nlim( 0 ),
  m_nchare( static_cast< std::size_t >( nchare ) ),
  m_msum( msum ),
  m_bid( bid ),
  m_lid( lid ),
  m_inpoel( inpoel ),
  m_fluxcorrector( m_inpoel.size() ),
  m_p( nu, np*2 ),
  m_q( nu, np*2 ),
  m_a( nu, np ),
  m_pc(),
  m_qc(),
  m_ac(),
  m_ul(),
  m_dul(),
  m_du()
// *****************************************************************************
//  Constructor
//! \param[in] nchare Total number of worker chares
//! \param[in] nu Number of unknowns in solution vector
//! \param[in] np Total number of properties, i.e., scalar variables or
//!   components, per unknown in solution vector
//! \param[in] msum Global mesh node IDs associated to chare IDs bordering the
//!   mesh chunk we operate on
//! \param[in] bid Local chare-boundary mesh node IDs at which we receive
//!   contributions associated to global mesh node IDs of mesh elements we
//!   contribute to
//! \param[in] lid Local mesh node ids associated to the global ones of owned
//!   elements
//! \param[in] inpoel Mesh connectivity of our chunk of the mesh
// *****************************************************************************
{
  // Allocate receive buffers for FCT
  m_pc.resize( m_bid.size() );
  for (auto& b : m_pc) b.resize( np*2 );
  m_qc.resize( m_bid.size() );
  for (auto& b : m_qc) b.resize( np*2 );
  m_ac.resize( m_bid.size() );
  for (auto& b : m_ac) b.resize( np );
}

tk::Fields
DistFCT::lump( const Discretization& d )
// *****************************************************************************
//  Compute lumped mass lhs required for the low order solution
//! \param[in] d Discretization proxy to read mesh data from
//! \return Lumped mass matrix
// *****************************************************************************
{
  return m_fluxcorrector.lump( d.Coord(), m_inpoel );
}

tk::Fields
DistFCT::diff( const Discretization& d, const tk::Fields& Un )
// *****************************************************************************
//  Compute mass diffusion rhs contribution required for the low order solution
//! \param[in] d Discretization proxy to read mesh data from
//! \param[in] Un Solution at the previous time step
//! \return Lumped mass matrix
// *****************************************************************************
{
  return m_fluxcorrector.diff( d.Coord(), m_inpoel, Un );
}

void
DistFCT::next()
// *****************************************************************************
// Prepare for next time step stage
// *****************************************************************************
{
  #ifndef NDEBUG
  thisProxy[ thisIndex ].wait4ver();
  #endif
  thisProxy[ thisIndex ].wait4fct();
  thisProxy[ thisIndex ].wait4app();

  // Initialize FCT data structures for new time step
  m_p.fill( 0.0 );
  m_a.fill( 0.0 );
  for (std::size_t p=0; p<m_a.nunk(); ++p)
    for (ncomp_t c=0; c<g_inputdeck.get<tag::component>().nprop(); ++c) {
      m_q(p,c*2+0,0) = -std::numeric_limits< tk::real >::max();
      m_q(p,c*2+1,0) = std::numeric_limits< tk::real >::max();
    }

  for (auto& b : m_pc) std::fill( begin(b), end(b), 0.0 );
  for (auto& b : m_ac) std::fill( begin(b), end(b), 0.0 );
  for (auto& b : m_qc)
    for (ncomp_t c=0; c<m_a.nprop(); ++c) {
      b[c*2+0] = -std::numeric_limits< tk::real >::max();
      b[c*2+1] = std::numeric_limits< tk::real >::max();
    }
}

void
DistFCT::aec( const Discretization& d,
              const tk::Fields& dUh,
              const tk::Fields& Un,
              const std::unordered_map< std::size_t,
                      std::vector< std::pair< bool, tk::real > > >& bc )
// *****************************************************************************
//  Compute and sum antidiffusive element contributions (AEC) to mesh nodes
//! \param[in] d Discretization proxy to read mesh data from
//! \param[in] dUh Increment of the high order solution
//! \param[in] Un Solution at the previous time step
//! \param[in] bc Vector of pairs of bool and boundary condition value
//!   associated to mesh node IDs at which to set Dirichlet boundary conditions.
//!   Note that this BC data structure must include boundary conditions set
//!   across all PEs, not just the ones need to be set on this PE.
//! \details This function computes and starts communicating m_p, which stores
//!    the sum of all positive (negative) antidiffusive element contributions to
//!    nodes (Lohner: P^{+,-}_i), see also FluxCorrector::aec().
// *****************************************************************************
{
  // Store a copy of the high order solution increment for later
  m_du = dUh;

  // Compute and sum antidiffusive element contributions to mesh nodes. Note
  // that the sums are complete on nodes that are not shared with other chares
  // and only partial sums on chare-boundary nodes.
  m_fluxcorrector.aec(d.Coord(), m_inpoel, d.Vol(), bc, d.Gid(), dUh, Un, m_p);

  if (d.Msum().empty())
    comaec_complete();
  else // send contributions to chare-boundary nodes to fellow chares
    for (const auto& n : d.Msum()) {
      std::vector< std::vector< tk::real > > p;
      for (auto i : n.second) p.push_back( m_p[ tk::cref_find(m_lid,i) ] );
      thisProxy[ n.first ].comaec( n.second, p );
    }

  ownaec_complete();
  #ifndef NDEBUG
  ownaec_complete();
  #endif
}

void
DistFCT::comaec( const std::vector< std::size_t >& gid,
                 const std::vector< std::vector< tk::real > >& P )
// *****************************************************************************
//  Receive sums of antidiffusive element contributions on chare-boundaries
//! \param[in] gid Global mesh node IDs at which we receive AEC contributions
//! \param[in] P Partial sums of positive (negative) antidiffusive element
//!   contributions to chare-boundary nodes
//! \details This function receives contributions to m_p, which stores the
//!   sum of all positive (negative) antidiffusive element contributions to
//!   nodes (Lohner: P^{+,-}_i), see also FluxCorrector::aec(). While m_p stores
//!   own contributions, m_pc collects the neighbor chare contributions during
//!   communication. This way work on m_p and m_pc is overlapped. The two are
//!   combined in lim().
// *****************************************************************************
{
  Assert( P.size() == gid.size(), "Size mismatch" );

  using tk::operator+=;

  for (std::size_t i=0; i<gid.size(); ++i) {
    auto bid = tk::cref_find( m_bid, gid[i] );
    Assert( bid < m_pc.size(), "Indexing out of bounds" );
    m_pc[ bid ] += P[i];
  }

  if (++m_naec == m_msum.size()) {
    m_naec = 0;
    comaec_complete();
  }
}

void
DistFCT::alw( const tk::Fields& Un,
              const tk::Fields& Ul,
              const tk::Fields& dUl,
              const SchemeProxy& scheme )
// *****************************************************************************
//  Compute the maximum and minimum unknowns of elements surrounding nodes
//! \param[in] Un Solution at the previous time step
//! \param[in] Ul Low order solution
//! \param[in] dUl Low order solution increment
//! \param[in] scheme Discretization scheme Charm++ proxy we interoperate with
//! \details This function computes and starts communicating m_q, which stores
//!    the maximum and mimimum unknowns of all elements surrounding each node
//!    (Lohner: u^{max,min}_i), see also FluxCorrector::alw().
// *****************************************************************************
{
  // Store a copy of the low order solution vector and its increment for later
  m_ul = Ul;
  m_dul = dUl;

  // Store discretization scheme proxy
  m_scheme = scheme;

  // Compute the maximum and minimum unknowns of all elements surrounding nodes
  // Note that the maximum and minimum unknowns are complete on nodes that are
  // not shared with other chares and only partially complete on chare-boundary
  // nodes.
  m_fluxcorrector.alw( m_inpoel, Un, Ul, m_q );

  if (m_msum.empty())
    comalw_complete();
  else // send contributions at chare-boundary nodes to fellow chares
    for (const auto& n : m_msum) {
      std::vector< std::vector< tk::real > > q;
      for (auto i : n.second) q.push_back( m_q[ tk::cref_find(m_lid,i) ] );
      thisProxy[ n.first ].comalw( n.second, q );
    }

  ownalw_complete();
  #ifndef NDEBUG
  ownalw_complete();
  #endif
}

void
DistFCT::comalw( const std::vector< std::size_t >& gid,
                 const std::vector< std::vector< tk::real > >& Q )
// *****************************************************************************
// Receive contributions to the maxima and minima of unknowns of all elements
// surrounding mesh nodes on chare-boundaries
//! \param[in] gid Global mesh node IDs at which we receive contributions
//! \param[in] Q Partial contributions to maximum and minimum unknowns of all
//!   elements surrounding nodes to chare-boundary nodes
//! \details This function receives contributions to m_q, which stores the
//!   maximum and mimimum unknowns of all elements surrounding each node
//!   (Lohner: u^{max,min}_i), see also FluxCorrector::alw(). While m_q stores
//!   own contributions, m_qc collects the neighbor chare contributions during
//!   communication. This way work on m_q and m_qc is overlapped. The two are
//!   combined in lim().
// *****************************************************************************
{
  Assert( Q.size() == gid.size(), "Size mismatch" );

  for (std::size_t i=0; i<gid.size(); ++i) {
    auto bid = tk::cref_find( m_bid, gid[i] );
    Assert( bid < m_qc.size(), "Indexing out of bounds" );
    auto& o = m_qc[ bid ];
    const auto& q = Q[i];
    for (ncomp_t c=0; c<m_a.nprop(); ++c) {
      if (q[c*2+0] > o[c*2+0]) o[c*2+0] = q[c*2+0];
      if (q[c*2+1] < o[c*2+1]) o[c*2+1] = q[c*2+1];
    }
  }

  if (++m_nalw == m_msum.size()) {
    m_nalw = 0;
    comalw_complete();
  }
}

void
DistFCT::lim()
// *****************************************************************************
//  Compute the limited antidiffusive element contributions
//! \details This function computes and starts communicating m_a, which stores
//!   the limited antidiffusive element contributions assembled to nodes
//!   (Lohner: AEC^c), see also FluxCorrector::limit().
// *****************************************************************************
{
  // Combine own and communicated contributions to P and Q
  for (const auto& b : m_bid) {
    auto lid = tk::cref_find( m_lid, b.first );
    const auto& bpc = m_pc[ b.second ];
    const auto& bqc = m_qc[ b.second ];
    for (ncomp_t c=0; c<m_p.nprop()/2; ++c) {
      m_p(lid,c*2+0,0) += bpc[c*2+0];
      m_p(lid,c*2+1,0) += bpc[c*2+1];
      if (bqc[c*2+0] > m_q(lid,c*2+0,0)) m_q(lid,c*2+0,0) = bqc[c*2+0];
      if (bqc[c*2+1] < m_q(lid,c*2+1,0)) m_q(lid,c*2+1,0) = bqc[c*2+1];
    }
  }

  m_fluxcorrector.lim( m_inpoel, m_p, m_ul, m_q, m_a );

  if (m_msum.empty())
    comlim_complete();
  else // send contributions to chare-boundary nodes to fellow chares
    for (const auto& n : m_msum) {
      std::vector< std::vector< tk::real > > a;
      for (auto i : n.second) a.push_back( m_a[ tk::cref_find(m_lid,i) ] );
      thisProxy[ n.first ].comlim( n.second, a );
    }

  ownlim_complete();
}

void
DistFCT::comlim( const std::vector< std::size_t >& gid,
                 const std::vector< std::vector< tk::real > >& A )
// *****************************************************************************
//  Receive contributions of limited antidiffusive element contributions on
//  chare-boundaries
//! \param[in] gid Global mesh node IDs at which we receive contributions
//! \param[in] A Partial contributions to antidiffusive element contributions to
//!   chare-boundary nodes
//! \details This function receives contributions to m_a, which stores the
//!   limited antidiffusive element contributions assembled to nodes (Lohner:
//!   AEC^c), see also FluxCorrector::limit(). While m_a stores own
//!   contributions, m_ac collects the neighbor chare contributions during
//!   communication. This way work on m_a and m_ac is overlapped. The two are
//!   combined in apply().
// *****************************************************************************
{
  Assert( A.size() == gid.size(), "Size mismatch" );

  using tk::operator+=;

  for (std::size_t i=0; i<gid.size(); ++i) {
    auto bid = tk::cref_find( m_bid, gid[i] );
    Assert( bid < m_ac.size(), "Indexing out of bounds" );
    m_ac[ bid ] += A[i];
  }
 
  if (++m_nlim == m_msum.size()) {
    m_nlim = 0;
    comlim_complete();
  }
}

void
DistFCT::verify()
// *****************************************************************************
// Verify antidiffusive element contributions up to linear solver convergence
// *****************************************************************************
{
  m_fluxcorrector.verify( m_nchare, m_inpoel, m_du, m_dul );
}

bool
DistFCT::verifyBC()
// *****************************************************************************
//  Verify that the change in the solution at those nodes where Dirichlet
//  boundary conditions are set is exactly the amount the BCs prescribe
//! \return True if the solution is correct at Dirichlet boundary condition
//!   nodes
// *****************************************************************************
{
  // Call Scheme.correctBC(). The code below is equivalent to the function call
  // m_scheme[ thisIndex ].ckLocal()->correctBC( m_a ). The call is done via
  // a variant to facilitate calling back to chare arrays of different types,
  // e.g., MatCG or DiagCG. See also DistFCT::SchemeProxy.
  auto e = tk::element< ProxyElem >( m_scheme, thisIndex );
  return boost::apply_visitor( correctBC(m_a), e );
}

void
DistFCT::apply()
// *****************************************************************************
// Apply limited antidiffusive element contributions
// *****************************************************************************
{
  // Combine own and communicated contributions to A
  for (const auto& b : m_bid) {
    auto lid = tk::cref_find( m_lid, b.first );
    const auto& bac = m_ac[ b.second ];
    for (ncomp_t c=0; c<m_a.nprop(); ++c) m_a(lid,c,0) += bac[c];
  }

  // Verify that solution values do not change at Dirichlet BC nodes
  Assert( verifyBC(), "Dirichlet boundary condition incorrect" );

  // Prepare for next time step. The code below is equivalent to the function
  // call m_scheme[ thisIndex ].ckLocal()->next( m_a ). The call is done via a
  // variant to facilitate calling back to chare arrays of different types,
  // e.g., MatCG or DiagCG. See also DistFCT::SchemeProxy.
  auto e = tk::element< ProxyElem >( m_scheme, thisIndex );
  boost::apply_visitor( Next(m_a), e );
}

#include "NoWarning/distfct.def.h"
