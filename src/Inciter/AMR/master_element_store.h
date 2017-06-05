#ifndef AMR_master_element_store_h
#define AMR_master_element_store_h

#include <map>
#include "Refinement_State.h"

namespace AMR {

    class master_element_store_t {
        private:
            std::map<size_t, Refinement_State> master_elements;
        public:
            /**
             * @brief Add an element to the master element list.
             *
             * @param element_number The element number to add (currently
             * somewhat redundant?)
             * @param refinement_case The refinement_case which gave rise to
             * this element
             * @param refinement_level The level of refinement
             * @param parent_id The id of the parent element
             *
             * @return The id of the added element
             */
            size_t add
            (
                 size_t element_number,
                 Refinement_Case refinement_case,
                 size_t refinement_level,
                 size_t parent_id
            )
            {
                Refinement_State data = Refinement_State(
                        element_number,
                        refinement_case,
                        refinement_level,
                        parent_id
                );

                master_elements.insert( std::pair<size_t, Refinement_State>(element_number, data));

                return element_number;
            }

            /**
             * @brief Add a master element, with a default parent_id (0) and
             * default refinement level (0)
             *
             * @param element_number The element number to add
             * @param refinement_case The refinement_case which gave rise to
             * this element
             *
             * @return The id of the added element
             */
            size_t add(
                 size_t element_number,
                 Refinement_Case refinement_case
            )
            {
                add(element_number, refinement_case, 0, 0);
                return element_number;
            }

            /**
             * @brief Add a master element, with a specified parent_id which is
             * used to udpate the refinement_level
             *
             * @param element_number The element number to add
             * @param refinement_case The refinement_case which gave rise to
             * this element
             * @param parent_id The id of the parent element
             *
             * @return The id of the added element
             */
            size_t add(
                 size_t element_number,
                 Refinement_Case refinement_case,
                 size_t parent_id
            )
            {
                size_t refinement_level =
                    get(parent_id).refinement_level + 1;

                trace_out << "Refinement Level " << refinement_level <<
                    std::endl;

                add(element_number, refinement_case,
                        refinement_level, parent_id);

                return element_number;
            }

            /**
             * @brief Accessor method to retrieve master element by element id
             *
             * @param element_id The element_id of the master_element to fetch
             *
             * @return The master_element which represents the corresponding tet
             */
            Refinement_State& get(size_t element_id)
            {
                return master_elements.at(element_id);
            }

            // TODO: document this
            void erase(size_t id) {
                master_elements.erase(id);
            }

            size_t get_parent(size_t id)
            {
                return get(id).parent_id;
            }

            /**
             * @brief Wrapper function to calculate number of master elements
             *
             * @return total number of master elements
             */
            size_t size() {
                return master_elements.size();
            }

    };
}

#endif // guard
