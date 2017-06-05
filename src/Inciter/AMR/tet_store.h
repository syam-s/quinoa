#ifndef AMR_tet_store_h
#define AMR_tet_store_h

#include <set>
#include <vector>

#include "types.h"
#include "active_element_store.h"
#include "master_element_store.h"
#include "edge_store.h"
#include "util.h"

namespace AMR {

    class tet_store_t {
        private:
            // TODO: Remove this data structure!
            // This is a horrendous code abuse, and I'm sorry. I'm fairly
            // certain we'll be re-writing how this detection is done and just
            // wanted a quick-fix so I could move on :(
            std::set<size_t> center_tets; // Store for 1:4 centers

            AMR::active_element_store_t active_elements;
            AMR::master_element_store_t master_elements;

            std::vector<real_t> cell_type_list;
            std::vector<real_t> refinement_level_list;

            std::vector< std::size_t > active_tetinpoel;
            std::set< std::size_t > active_nodes;

            // TODO: I'd like this on the stack, but still pass an initial val
            AMR::id_generator_t id_generator;

        public:
            // Public so it can be trivially grabbed for looping over.
            // TODO: implement iterators at some point..
            tet_list_t tets;
            AMR::edge_store_t edge_store;

            // TODO: Make this private at some point
            AMR::marked_refinements_store_t marked_refinements;

            // TODO: Document this
            size_t size() {
                return tets.size();
            }

            bool is_active(size_t id)
            {
                return active_elements.exists(id);
            }

            Refinement_Case get_refinement_case(size_t id)
            {
                return data(id).refinement_case;
            }

            Refinement_State& data(size_t id)
            {
                return master_elements.get(id);
            }

            /**
             * @brief Method to insert tet into the tet store, so the
             * underlying data structure doesn't have to be interfaced with
             * directly
             *
             * @param id Id of the added tet
             * @param t The tet element
             */
            // TODO: Rename this?
            void insert(size_t id, tet_t t)
            {
                assert( !exists(id) );
                tets.insert( std::pair<size_t, tet_t>(id, t));
            }

            /**
             * @brief Getter for tet element
             *
             * @param id Id of tet to get
             *
             * @return Copy of the tet
             */
            tet_t get( size_t id )
            {
                assert( exists(id) );
                return tets.at(id);
            }

            /**
             * @brief Function to check if a tet exists. Useful for debugging
             * access to invalid tets, or trying to re-create a tet which
             * already exists
             *
             * @param id Id of the tet to check
             *
             * @return Bool stating if the tet already exists
             */
            bool exists(size_t id)
            {
                auto f = tets.find(id);
                if (f != tets.end())
                {
                    trace_out << "tet " << id << " exists." << std::endl;
                    return true;
                }
                return false;
            }

            /**
             * @brief Function to store a tet from a list of nodes
             *
             * @param nodes The node ids which make up the tet
             */
            void store_tet(size_t id, tet_t nodes) {

                insert(id, nodes);

                // Sanity check the storage ids
                // (this is probably better in a function/2d loop)
                assert( nodes[0] != nodes[1] );
                assert( nodes[0] != nodes[2] );
                assert( nodes[0] != nodes[3] );
                assert( nodes[1] != nodes[2] );
                assert( nodes[1] != nodes[3] );
                assert( nodes[2] != nodes[3] );
            }

            /**
             * @brief Convenience function to store a tet without first building
             * a list
             *
             * @param first First Node
             * @param second Second Node
             * @param third Third Node
             * @param forth Forth Node
             */
            void store_tet(
                    size_t id,
                    size_t first,
                    size_t second,
                    size_t third,
                    size_t forth
                    )
            {
                store_tet( id, { {first, second, third, forth} } );
            }

            void add(
                    size_t id,
                    tet_t nodes,
                    Refinement_Case refinement_case,
                    size_t parent_id)
            {

                add(id, nodes, refinement_case);

                // Deal with updating parent

                // TODO: Is it bad form the reach through an object like that? (Law of Demeter)
                master_elements.get(parent_id).children.push_back(id);

                master_elements.get(parent_id).num_children++;
                assert( master_elements.get(parent_id).num_children <= 8);

                trace_out << "Added child " << id << " (" <<
                    master_elements.get(parent_id).num_children << ")" << std::endl;
            }

            /**
             * @brief Convenience function to add a tet to the master_elements
             * and active_elements store
             *
             * @param nodes A list of the nodes which form th etet
             * @param refinement_case The refinement case which caused this tet
             * to be generated
             */
            void add(size_t id, tet_t nodes, Refinement_Case refinement_case) {

                store_tet(id, nodes);

                // Add to master list
                master_elements.add(id, refinement_case, 0, 0);

                // The new master element should start as active
                active_elements.add(id);
            }

            void add(tet_t nodes, Refinement_Case refinement_case) {

                size_t id = id_generator.get_next_tet_id();

                store_tet(id, nodes);

                // Add to master list
                master_elements.add(id, refinement_case, 0, 0);

                // The new master element should start as active
                active_elements.add(id);
            }

            void add(
                    size_t id,
                    size_t first,
                    size_t second,
                    size_t third,
                    size_t forth,
                    Refinement_Case refinement_case,
                    size_t parent_id
                    )
            {
                return add(
                        id,
                        { {first, second, third, forth} },
                        refinement_case,
                        parent_id
                        );

            }

            // NOTE: this does *not* deal with edges
            /**
             * @brief Function to delete a tet from the tet store (useful in
             * derefinement)
             *
             * @param id id of the tet to delete
             */
            void erase(size_t id)
            {
                deactivate(id);
                master_elements.erase(id);
                tets.erase(id);
                // TODO: Should this update the number of children here rather than at the call site?
            }

            /**
             * @brief Function to remove a tet from the active tet list
             *
             * @param id The id of the tet to deactivate
             */
            void deactivate(size_t id) {
                active_elements.erase(id);
                // TODO: For safety, should we also mark it's edges as not
                // needing to be refined?
            }

            void activate(size_t id) {
                if (!is_active(id) )
                {
                    active_elements.add(id);
                }
            }

            /**
             * @brief Function to add a tet to a list which maintains what is a
             * center tet. (The need to maintain a list could be replaced by a
             * geometric check on the tet itself)
             *
             * @param id Id of the tet to add
             */
            void add_center(size_t id)
            {
                assert( !is_center(id) );
                center_tets.insert(id);
            }

            /**
             * @brief function to check if a tet is a center tet in a 1:4
             *
             * @param id Id of the tet to check
             *
             * @return Bool stating if it's a center tet or not
             */
            bool is_center(size_t id)
            {
                if (center_tets.find(id) != center_tets.end())
                {
                    return true;
                }
                return false;
            }

            /**
             * @brief Function to get a list of refinement levels, useful for
             * vis
             *
             * @return Vector containing refinement levels of tets
             */
            std::vector< real_t >& get_refinement_level_list()
            {
                refinement_level_list.clear();

                for (const auto& kv : tets)
                {
                    size_t element_id = kv.first;
                    if (active_elements.exists( element_id  )) {
                        real_t val = master_elements.get(element_id).refinement_level;
                        refinement_level_list.push_back(val);
                    }
                }

                trace_out << "Made refinement level list of len " << refinement_level_list.size() << std::endl;
                return refinement_level_list;
            }

            /**
             * @brief Function to return a list of cell types, useful when
             * invoking the vis to do coloring by cell type
             *
             * @return Vector listening the types of cells
             */
            std::vector< real_t >& get_cell_type_list()
            {
                cell_type_list.clear();

                for (const auto& kv : tets)
                {
                    size_t element_id = kv.first;

                    if (active_elements.exists( element_id  )) {

                        real_t val = 0.0;

                        // Be a good citizen, make this enum human readable
                        switch (master_elements.get(element_id).refinement_case)
                        {
                            case Refinement_Case::one_to_two:
                                val = 2.0;
                                break;
                            case Refinement_Case::one_to_four:
                                val = 4.0;
                                break;
                            case Refinement_Case::one_to_eight:
                                val = 8.0;
                                break;
                            case Refinement_Case::initial_grid:
                                val = 1.0;
                                break;
                            default:
                                val = -1.0;
                        }
                        cell_type_list.push_back(val);
                    }
                }

                trace_out << "Made cell type list of len " << cell_type_list.size() << std::endl;
                return cell_type_list;
            }

            /**
             * @brief Function to extract only the active elements from tetinpeol
             *
             * @return List of only active elements
             */
            // TODO: need equiv for m_x/m_y/m_z? Otherwise there will be
            // useless nodes in m_x etc? (Although that's functionally fine)
            std::vector< std::size_t >& get_active_inpoel()
            {
                active_tetinpoel.clear();
                active_nodes.clear();

                for (const auto& kv : tets)
                {

                    size_t element_id = kv.first;
                    auto t = kv.second;

                    if (is_active( element_id )) {
                        active_tetinpoel.push_back( t[0] );
                        active_tetinpoel.push_back( t[1] );
                        active_tetinpoel.push_back( t[2] );
                        active_tetinpoel.push_back( t[3] );

                        active_nodes.insert( t[0] );
                        active_nodes.insert( t[1] );
                        active_nodes.insert( t[2] );
                        active_nodes.insert( t[3] );
                    }
                }

                return active_tetinpoel;
            }

            // TODO: Remove this as it shouldn't make a difference..but it's good to check
            std::vector< real_t > get_active_coordinate_array(std::vector<real_t> in)
            {
                std::vector< real_t > active;

                for (size_t i = 0; i < in.size(); i++)
                {
                    // If an active tet has an edge which contains this i value
                    if ( active_nodes.find(i) != active_nodes.end() )
                    {
                        active.push_back( in[i] );
                    }
                }

                trace_out << "in size " << in.size() << " active size " << active.size() << std::endl;
                return active;
            }

            // TODO: These mark methods can probably be a single one to which a
                // Refinement_Case is passed, depending on how extra edges are marked
            /**
             * @brief Function to mark a given tet as needing a 1:2 refinement
             *
             * @param tet_id The tet to mark
             */
            void mark_one_to_two(size_t tet_id)
            {
                // TODO: If none of these methods need extra markings, then
                // change them into a single method
                // TODO: Mark extra edges
                trace_out << "Mark " << tet_id << " as 1:2" << std::endl;
                marked_refinements.add(tet_id, Refinement_Case::one_to_two);
            }

            /**
             * @brief Function to mark a given tet as needing a 1:4 refinement
             *
             * @param tet_id The tet to mark
             */
            void mark_one_to_four(size_t tet_id)
            {
                // TODO: Mark extra edges
                trace_out << "Mark " << tet_id << " as 1:4" << std::endl;
                marked_refinements.add(tet_id, Refinement_Case::one_to_four);
            }

            // TODO: Document this
            void mark_two_to_eight(size_t tet_id)
            {
                // TODO: Mark extra edges
                trace_out << "Mark " << tet_id << " as 2:8" << std::endl;
                marked_refinements.add(tet_id, Refinement_Case::two_to_eight);
            }

            // TODO: Document this
            void mark_four_to_eight(size_t tet_id)
            {
                // TODO: Mark extra edges
                trace_out << "Mark " << tet_id << " as 4:8" << std::endl;
                marked_refinements.add(tet_id, Refinement_Case::four_to_eight);
            }

            /**
             * @brief Function to mark a given tet as needing a 1:8 refinement
             *
             * @param tet_id The tet to mark
             */
            void mark_one_to_eight(size_t tet_id)
            {
                // TODO: Mark extra edges
                trace_out << "Mark " << tet_id << " as 1:8" << std::endl;
                marked_refinements.add(tet_id, Refinement_Case::one_to_eight);
            }

            bool has_refinement_decision(size_t id)
            {
                return marked_refinements.exists(id);
            }

            /**
             * @brief Helper debug function to print tet information
             */
            void print_tets() {
                for (const auto& kv : tets)
                {
                    tet_t tet = kv.second;

                    trace_out << "Tet " << kv.first << " has edges :" <<
                        tet[0] << ", " <<
                        tet[1] << ", " <<
                        tet[2] << ", " <<
                        tet[3] << ", " <<
                    std::endl;
                }
            }

            void print_edges()
            {
                edge_store.print();
            }

            void print_node_types()
            {

                int initial_grid = 0;
                int one_to_two = 0;
                int one_to_four = 0;
                int one_to_eight = 0;
                int unknown = 0;

                for (const auto& kv : tets)
                {
                    size_t tet_id = kv.first;
                    if (is_active(tet_id))
                    {
                        switch(get_refinement_case(tet_id))
                        {
                            case Refinement_Case::one_to_two:
                                one_to_two++;
                                break;
                            case Refinement_Case::one_to_four:
                                one_to_four++;
                                break;
                            case Refinement_Case::one_to_eight:
                                one_to_eight++;
                                break;
                            case Refinement_Case::initial_grid:
                                initial_grid++;
                                break;
                            default:
                                unknown++;
                        }

                    }
                }

                std::cout << "Active Totals:" << std::endl;
                std::cout << "  --> Initial = " << initial_grid << std::endl;
                std::cout << "  --> 1:2 = " << one_to_two << std::endl;
                std::cout << "  --> 1:4 = " << one_to_four << std::endl;
                std::cout << "  --> 1:8 = " << one_to_eight << std::endl;
            }

            edge_list_t generate_edge_keys(size_t tet_id)
            {
                tet_t tet = get(tet_id);
                return edge_store.generate_keys(tet);
            }

            void generate_edges(size_t i) {
                //  For tet ABCD, edges are:
                //  AB, AC, AD, BC, BD, CD
                //
                edge_list_t edge_list = generate_edge_keys(i);

                for (size_t j = 0; j < NUM_TET_EDGES; j++)
                {
                    std::string key = edge_list[j];
                    std::vector<std::string> nodes = util::split(key,KEY_DELIM);

                    size_t A =  std::stoul (nodes[0],nullptr,0);
                    size_t B =  std::stoul (nodes[1],nullptr,0);

                    Edge_Refinement er = Edge_Refinement(A, B, 0.0, false,
                            false, false, Edge_Lock_Case::unlocked);

                    edge_store.add(key, er);
                }
            }

            /**
             * @brief function to generate edge_ids (not strings) for
             * a given tet_id
             *
             * @param tet The tet_id for which the edge_ids will be generated
             *
             * @return The edge_list_ids_t of edge ids
             */
            edge_list_ids_t generate_edge_ids(size_t tet) {
                edge_list_t edge_list = generate_edge_keys(tet);
                return AMR::edge_store_t::generate_edge_ids_from_edge_list(edge_list);
            }

            /**
             * @brief function to take a tet_id, finds it's nodes, and
             * expresses them as faces
             *
             * Take tet ABCD, generate faces {ABC, ABD, ACD, BCD}
             *
             * @param tet_id The tet to generate faces for
             *
             * @return A list of faces making this tet
             */
            face_list_t generate_face_lists(size_t tet_id)
            {
                // Hard code this for now...
                tet_t tet = get(tet_id);

                trace_out  << "Tet has nodes " <<
                    tet[0] << ", " <<
                    tet[1] << ", " <<
                    tet[2] << ", " <<
                    tet[3] << ", " <<
                    std::endl;

                face_list_t face_list;

                // ABC
                face_list[0][0] = tet[0];
                face_list[0][1] = tet[1];
                face_list[0][2] = tet[2];

                // ABD
                face_list[1][0] = tet[0];
                face_list[1][1] = tet[1];
                face_list[1][2] = tet[3];

                // ACD
                face_list[2][0] = tet[0];
                face_list[2][1] = tet[2];
                face_list[2][2] = tet[3];

                // BCD
                face_list[3][0] = tet[1];
                face_list[3][1] = tet[2];
                face_list[3][2] = tet[3];

                return face_list;
            }

            /**
             * @brief Function which marks all edges in a given tet as needing
             * to be refined
             *
             * @param tet_id ID of the tet to mark
             */
            void mark_edges_for_refinement(size_t tet_id)
            {
                edge_list_t edge_list = generate_edge_keys(tet_id);
                for (size_t k = 0; k < NUM_TET_EDGES; k++)
                {
                    std::string key = edge_list[k];
                    edge_store.mark_for_refinement(key);
                    trace_out << "Marking edge " << key << " for refine " << std::endl;
                }
            }

            /**
             * @brief Delete existing edges. Iterate over the tets, and add them to
             * the edge store.
             */
            // TODO: Rename this
            void generate_edges() {

                // Go over tets, and generate all known edges
                edge_store.edges.clear();

                // Jump over tets
                for (const auto& kv : tets)
                {
                    size_t tet_id = kv.first;
                    generate_edges(tet_id);
                }
            }

            void unset_marked_children(size_t parent_id)
            {
                Refinement_State& parent = data(parent_id);
                for (auto c : parent.children)
                {
                    marked_refinements.erase(c);
                }
            }


    };
}

#endif // guard
