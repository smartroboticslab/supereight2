/*
 * SPDX-FileCopyrightText: 2021-2022 Smart Robotics Lab, Imperial College London, Technical University of Munich
 * SPDX-FileCopyrightText: 2021-2022 Nils Funk
 * SPDX-FileCopyrightText: 2021-2022 Sotiris Papatheodorou
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SE_DATA_ID_HPP
#define SE_DATA_ID_HPP

#include <se/map/utils/setup_util.hpp>
#include <se/map/utils/type_util.hpp>

namespace se {

template<Id IdB>
struct IdData {
    struct Config {
        void readYaml(const std::string& /* yaml_file */)
        {
        }
    };
};

template<>
struct IdData<Id::On> {
    id_t id = g_no_id;

    /** Set the ID to \p id if \p id is non-zero and return whether the data was updated. */
    bool update(const id_t id);

    struct Config {
        /** Reads the struct members from the "data" node of a YAML file. Members not present in the
         * YAML file aren't modified.
         */
        void readYaml(const std::string& yaml_file);
    };
};

std::ostream& operator<<(std::ostream& os, const IdData<Id::Off>::Config& c);
std::ostream& operator<<(std::ostream& os, const IdData<Id::On>::Config& c);

} // namespace se

#include "impl/data_id_impl.hpp"

#endif // SE_DATA_ID_HPP
