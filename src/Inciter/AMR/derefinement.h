#ifndef AMR_derefinement_h
#define AMR_derefinement_h

#include "tet_store.h"

// TODO: make this have a base class to support multiple generator schemes
// using the policy design pattern
namespace AMR {

    class derefinement_t {
        private:

            // TODO: Tidy up edge store references here 
            tet_store_t* tet_store;

        public:

            const size_t DEFAULT_REFINEMENT_LEVEL = 0; //TODO: Is this in the right place?
            const size_t MIN_REFINEMENT_LEVEL = DEFAULT_REFINEMENT_LEVEL;

            derefinement_t(tet_store_t* ts) : tet_store(ts)
            {
            }

            /**
             * @brief Function to iterate over children and remove them
             *
             * @param parent_id Id of the parent for whom you will delete the
             * children
             */
            void derefine_children(size_t parent_id)
            {
                // For a given tet_id, find and delete its children
                Refinement_State& parent = tet_store->data(parent_id);
                for (auto c : parent.children)
                {
                    trace_out << "Derefine child " << c << std::endl;

                    tet_store->erase(c);
                    parent.num_children--; // Could directly set to 0
                }
                parent.children.clear();
            }

            /**
             * @brief Common code for derefinement. Deactives the children and
             * actives the parent
             *
             * @param parent_id The id of the parent
             */
            void generic_derefine(size_t parent_id)
            {
                derefine_children(parent_id);
                tet_store->activate(parent_id);
            }

            // TODO: Document This.
            void derefine_two_to_one(size_t parent_id)
            {
                delete_intermediates_of_children(parent_id);
                generic_derefine(parent_id);
            }

            // TODO: Document This.
            void derefine_four_to_one(size_t parent_id)
            {
                delete_intermediates_of_children(parent_id);
                generic_derefine(parent_id);
            }

            // TODO: Document This.
            void derefine_eight_to_one(size_t parent_id)
            {
                generic_derefine(parent_id);

                // Delete the center edges
                    // If edge isn't in the parent, delete it? Is there a better way?
                edge_list_t parent_edges = tet_store->generate_edge_keys(parent_id);

                Refinement_State& parent = tet_store->data(parent_id);
                for (auto c : parent.children)
                {
                    edge_list_t child_edges = tet_store->generate_edge_keys(c);
                    delete_non_matching_edges( child_edges, parent_edges);
                }
            }

            // TODO: Document This.
            void derefine_four_to_two(size_t parent_id)
            {
                assert(0);
            }

            // TODO: Document This.
            void derefine_eight_to_two(size_t parent_id)
            {
                assert(0);
            }

            // TODO: Document This.
            void derefine_eight_to_four(size_t parent_id)
            {
                assert(0);
            }

            /**
             * @brief Loop over children and delete all intermediate edges
             *
             * @param parent_id Id of parent
             */
            void delete_intermediates_of_children(size_t parent_id)
            {
                Refinement_State& parent = tet_store->data(parent_id);
                for (auto c : parent.children)
                {
                    delete_intermediates(c);
                }
            }

            // TODO: Document this
            void delete_intermediates(size_t tet_id)
            {
                edge_list_t edge_list = tet_store->generate_edge_keys(tet_id);
                for (size_t k = 0; k < NUM_TET_EDGES; k++)
                {
                    std::string key = edge_list[k];
                    // accept this code may try delete an edge which has already gone
                    if (tet_store->edge_store.exists(key)) {
                        if (tet_store->edge_store.get(key).lockCase == Edge_Lock_Case::intermediate)
                        {
                            tet_store->edge_store.erase(key);
                        }
                    }
                }
            }

            /**
             * @brief If edge in candidate is not present in basis, delete the edge from
             * the main edge store
             *
             * @param candidate The edge list which is to be searched and deleted
             * @param basis The edge list to check against
             */
            void delete_non_matching_edges(edge_list_t candidate, edge_list_t basis)
            {
                trace_out << "Looking for edges to delete" << std::endl;

                // Loop over the edges in each child. Look over the basis and
                // if we can't find it, delete it
                for (size_t k = 0; k < NUM_TET_EDGES; k++)
                {
                    std::string search_key = candidate[k];

                    // Search the basis for it
                    bool found_it = false;

                    for (size_t l = 0; l < NUM_TET_EDGES; l++)
                    {
                        std::string key = basis[l];
                        if (search_key == key)
                        {
                            found_it = true;
                        }
                    }

                    // If we didn't find it, delete it
                    if (!found_it)
                    {
                        // Delete it
                        tet_store->edge_store.erase(search_key);
                    }
                }
            }


            /**
             * @brief function to detect when an invalid derefinement is
             * invoked
             *
             * @param tet_id Id the of the tet which will be de-refined
             *
             * @return A bool stating if the tet can be validly de-refined
             */
            bool check_allowed_derefinement(size_t tet_id)
            {
                Refinement_State& master_element = tet_store->data(tet_id);

                // Check this won't take us past the max refinement level
                if (master_element.refinement_level <= MIN_REFINEMENT_LEVEL)
                {
                    return false;
                }

                // If we got here, we didn't detect anything which tells us not
                // to
                return true;
            }


    };
}

#endif  // guard
