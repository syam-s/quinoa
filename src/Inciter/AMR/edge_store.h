#ifndef AMR_edge_store_h
#define AMR_edge_store_h

namespace AMR {

    class edge_store_t {
        public:
            std::map<std::string, Edge_Refinement> edges;

            size_t size()
            {
                return edges.size();
            }

            /**
             * @brief Function to create new edge between two nodes with an
             * intermediate. Given nodes A, B, and AB makes edge A->AB and AB->B
             *
             * @param A First end node
             * @param B Second end node
             * @param AB Intermediate node
             * @param lock_case Lock case for the new edges
             */
            void split(size_t A, size_t B, size_t AB, Edge_Lock_Case lock_case)
            {
                generate(A,AB, lock_case);
                generate(B,AB, lock_case);

                // Generate pertinent keys
                std::string keyAB = nodes_to_key(A, B);

                // NOTE: This isn't explicitly needed in the paper, and may be
                    // implicitly dealt with somewhere?
                //mark_edge_for_refinement(keyAB);
            }

            /**
             * @brief Given nodes A and B, generate an edge between them
             *
             * @param A First node
             * @param B Second node
             * @param lock_case Lock case for new edge
             */
            void generate(size_t A, size_t B, Edge_Lock_Case lock_case)
            {
                assert(A != B);
                // Generate key
                std::string keyAB = nodes_to_key(A, B);
                //Create refined edge
                Edge_Refinement edgeAB = Edge_Refinement(A, B, 0.00, false,
                        false, false, lock_case);
                // Add edge to store
                add(keyAB, edgeAB);
            }

            bool exists(std::string key)
            {
                if (edges.find(key) != edges.end())
                {
                    return true;
                }
                return false;
            }

            /**
             * @brief Function to retrieve an edge from the edge store
             *
             * @param key Key of the edge to get
             *
             * @return A reference to the fetched edge
             */
            Edge_Refinement& get(std::string key)
            {
                //trace_out << "get edge " << key << std::endl;
                assert( exists(key) );
                return edges.at(key);
            }

            Edge_Lock_Case lock_case(std::string key)
            {
                return get(key).lockCase;
            }

            void erase(std::string key)
            {
                edges.erase(key);
            }

            /**
             * @brief Function to add edge to ede store
             *
             * @param key The key for the given edge
             * @param e The edge data
             *
             * Note: This tolerate the addition of duplicate edges
             */
            void add(std::string key, Edge_Refinement e)
            {
                trace_out << "Adding edge " << key << std::endl;
                // Add edge if it doesn't exist (default behavior of insert)
                edges.insert( std::pair<std::string, Edge_Refinement>(key, e));

                // TODO: It may be worth adding a check here to ensure if we're
                // trying to add a new edge that exists it should contain the
                // same data
            }

            /**
             * @brief Function to build a  string key from two node ids
             * NOTE: Regardless of order of arguments, the same key will be generated
             */
            static std::string nodes_to_key(size_t A, size_t B)
            {
                return std::to_string(std::min(A,B)) + KEY_DELIM + std::to_string(std::max(A,B));
            }

            /**
             * @brief function to take the nodes representing a face
             * and to build the possible edges based on that
             *
             * For a given face {ABC}, generate the edge pairs {AB, AC, BC}
             *
             * @param face_ids The ids of the face to generate this for
             *
             * @return A (partially filled) list of all edges present on the
             * face
             */
            // FIXME: Is it OK that it leaves some of the array blank?
            static edge_list_t generate_keys_from_face_ids(face_ids_t face_ids)
            {
                edge_list_t key_list;
                size_t A = face_ids[0];
                size_t B = face_ids[1];
                size_t C = face_ids[2];

                std::string key = nodes_to_key(A,B);
                key_list[0] = key;

                key = nodes_to_key(A,C);
                key_list[1] = key;

                key = nodes_to_key(B,C);
                key_list[2] = key;

                return key_list;
            }

            /**
             * @brief function to take a list of edge and mark them all
             * as needing to be refined
             *
             * @param ids List of ids to mark for refinement
             */
            void mark_edges_for_refinement(std::vector<node_pair_t> ids) {
                for (size_t i = 0; i < ids.size(); i++)
                {
                    node_pair_t pair = ids[i];
                    std::string key = nodes_to_key(pair[0], pair[1]);
                    trace_out << "Marking " << key << std::endl;
                    mark_for_refinement(key);
                    trace_out << get(key).needs_refining << std::endl;
                }
            }


            /**
             * @brief function to mark a single edge as needing
             * refinement (provides a nice abstraction from messing with the
             * struct directly).
             *
             * @param key The edge key to mark as refinement
             */
            void mark_for_refinement(std::string key)
            {
                    assert( exists(key) );
                    get(key).needs_refining = true;
            }

            /**
             * @brief Function to unmark and edge as needing refinement
             *
             * @param key The key representing the edge to unmark
             */
            void unmark_for_refinement(std::string key)
            {
                    assert( exists(key) );
                    get(key).needs_refining = false;
            }

            // TODO: Document this (and implement!)
            void mark_edges_for_derefinement(std::vector<node_pair_t> ids) {
                for (size_t i = 0; i < ids.size(); i++)
                {
                    node_pair_t pair = ids[i];
                    std::string key = nodes_to_key(pair[0], pair[1]);
                    trace_out << "Marking " << key << std::endl;
                    mark_edge_for_derefinement(key);
                }
            }
            void mark_edge_for_derefinement(std::string key) {
                    get(key).needs_derefining = true;
            }


            /**
             * @brief Function to generate a list of edge keys from a tet
             *
             * @param tet The tet to generate edge pairs for
             *
             * @return A list (array) of edge keys which can be separated out to
             * name the two composing node ids
             */
            // TODO: Should this return a pointer/reference?
            edge_list_t generate_keys(tet_t tet)
            {
                // FIXME : Generate these with a (2d) loop and not hard code them?
                    // DRY THIS
                edge_list_t key_list;

                size_t A = tet[0];
                size_t B = tet[1];
                size_t C = tet[2];
                size_t D = tet[3];

                std::string key = "";

                key = nodes_to_key(A,B);
                key_list[0] = key;

                key = nodes_to_key(A,C);
                key_list[1] = key;

                key = nodes_to_key(A,D);
                key_list[2] = key;

                key = nodes_to_key(B,C);
                key_list[3] = key;

                key = nodes_to_key(B,D);
                key_list[4] = key;

                key = nodes_to_key(C,D);
                key_list[5] = key;

                return key_list;
            }

            /**
             * @brief function to take an edge_list_t (strings) and
             * convert them to ids (size_t)
             *
             * This is useful when the user wants the numerical
             * representation of the edge nodes not a string one
             *
             * @param edge_list A edge_list_t which represents an edge
             *
             * @return A size_t conversion of the inputted edge_list_t
             */
            static edge_list_ids_t generate_edge_ids_from_edge_list(edge_list_t edge_list) {
                edge_list_ids_t edge_list_ids;
                for (size_t i = 0; i < NUM_TET_EDGES; i++)
                {
                    edge_list_ids[i] = std::stoul (edge_list[i],nullptr,0);
                }
                return edge_list_ids;
            }

            /**
             * @brief Helper debug function to print edge information
             */
            void print() {
                for (const auto& kv : edges)
                {
                    trace_out << "edge " << kv.first << " between " <<
                        kv.second.A << " and " << kv.second.B << " val " <<
                        kv.second.refinement_criteria <<
                    std::endl;
                }
            }


    };
}

#endif // guard
