//
// Created by Robert Francis Bird on 6/1/17.
//

#ifndef QUINOA_MESH_ADAPTER_H
#define QUINOA_MESH_ADAPTER_H

#include "types.h"
#include "util.h"
#include "id_generator.h"

#include "marked_refinements_store.h"
#include "tet_store.h"

#include "node_connectivity.h"
#include "node_store.h"

#include "refinement.h"
#include "derefinement.h"

#include "Refinement_State.h"

namespace AMR {
    class mesh_adapter_t {

        public:
            AMR::tet_store_t tet_store;
            AMR::node_connectivity_t node_connectivity;

            // for coord type stuff
            AMR::node_store_t node_store;

            AMR::refinement_t *refiner;
            AMR::derefinement_t *derefiner;

            void init_node_store(coord_type* m_x, coord_type* m_y, coord_type* m_z, size_t* graph_size)
            {
                node_store.set_x(m_x);
                node_store.set_y(m_y);
                node_store.set_z(m_z);
                node_store.m_graphsize = graph_size;
            }

            void init_with_nodes(coord_type* m_x, coord_type* m_y, coord_type* m_z, size_t* graph_size)
            {
                init_node_store(m_x, m_y, m_z, graph_size);
                //init(); // TODO: This also needs to call init if you want node support to work
            }

            void init(std::vector<size_t> m_tetinpoel, size_t count) {
                node_connectivity.fill_initial_nodes(count);

                refiner = new AMR::refinement_t(&tet_store, &node_connectivity);
                derefiner = new AMR::derefinement_t(&tet_store);

                consume_tets(m_tetinpoel);
                tet_store.generate_edges();
            }

            // This would be a candidate for a nice design pattern with runtime
            // selectable functionality..
            // TODO: Document this
            void consume_tets( std::vector< std::size_t > m_tetinpoel )
            {
                for (size_t i = 0; i < m_tetinpoel.size(); i+=4)
                {
                    tet_t t = {
                        {m_tetinpoel[i],
                            m_tetinpoel[i+1],
                            m_tetinpoel[i+2],
                            m_tetinpoel[i+3]}
                    };

                    trace_out << "Consume tet " << i << std::endl;
                    tet_store.add(t, AMR::Refinement_Case::initial_grid);
                }
            }

            void uniform_refinement()
            {
                for (auto& kv : tet_store.edge_store.edges) {
                    kv.second.refinement_criteria = 1.0;
                }
                perform_refinement();
            }

            /**
             * @brief Function to call refinement after each tet has had it's
             * refinement case marked and calculated
             */
            void perform_refinement()
            {
                // Track tets which needs to be deleted this iteration
                std::set<size_t> round_two;

                trace_out << "Perform ref" << std::endl;

                // Do refinements
                for (const auto& kv : tet_store.tets)
                {
                    size_t tet_id = kv.first;
                    size_t parent_id = 0;

                    if (tet_store.has_refinement_decision(tet_id))
                    {
                        switch(tet_store.marked_refinements.get(tet_id))
                        {
                            case AMR::Refinement_Case::one_to_two:
                                refiner->refine_one_to_two(tet_id);
                                break;
                            case AMR::Refinement_Case::one_to_four:
                                refiner->refine_one_to_four(tet_id);
                                break;
                            case AMR::Refinement_Case::one_to_eight:
                                refiner->refine_one_to_eight(tet_id);
                                break;
                            case AMR::Refinement_Case::two_to_eight:
                                parent_id = AMR::id_generator_t::get_parent_id(tet_id);
                                round_two.insert(parent_id);
                                break;
                            case AMR::Refinement_Case::four_to_eight:
                                parent_id = AMR::id_generator_t::get_parent_id(tet_id);
                                round_two.insert(parent_id);
                                break;
                            case AMR::Refinement_Case::initial_grid:
                                // Do nothing
                            case AMR::Refinement_Case::none:
                                // Do nothing
                                break;
                            // No need for default as enum is explicitly covered
                        }
                        // Mark tet as not needing refinement
                        tet_store.marked_refinements.erase(tet_id);
                    }
                }

                for (const auto i : round_two)
                {
                    trace_out << "i " << i << std::endl;

                    AMR::Refinement_State& element = tet_store.data(i);

                    if (element.num_children == 2)
                    {
                        derefiner->derefine_two_to_one(i);
                    }
                    else if (element.num_children == 4)
                    {
                        derefiner->derefine_four_to_one(i);
                    }
                    else {
                        std::cout << "num children " << element.num_children << std::endl;
                        assert(0);
                    }

                    trace_out << "parent_id " << i << std::endl;
                    refiner->refine_one_to_eight(i);
                    tet_store.unset_marked_children(i); // FIXME: This will not work well in parallel
                    element.refinement_case = AMR::Refinement_Case::one_to_eight;
                }
                /*
                for (size_t t : tet_delete_list)
                {
                    // TODO: also decrease number of children
                    tets.erase(t);
                }
                */

                // Clean up dead edges
                // clean_up_dead_edges(); // Nothing get's marked as "dead" atm?

                std::cout << "Total Edges : " << tet_store.edge_store.size() << std::endl;
                std::cout << "Total Tets : " << tet_store.size() << std::endl;
                //std::cout << "Total Nodes : " << m_x.size() << std::endl;

                trace_out << "Done ref" << std::endl;
                tet_store.print_node_types();
                node_connectivity.print();
            }

    };
}

#endif //QUINOA_MESH_ADAPTER_H
