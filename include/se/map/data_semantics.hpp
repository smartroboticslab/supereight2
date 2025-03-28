/*
 * SPDX-FileCopyrightText: 2021-2022 Smart Robotics Lab, Imperial College London, Technical University of Munich
 * SPDX-FileCopyrightText: 2021-2022 Nils Funk
 * SPDX-FileCopyrightText: 2021-2022 Sotiris Papatheodorou
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SE_DATA_SEMANTICS_HPP
#define SE_DATA_SEMANTICS_HPP

#include <se/map/utils/setup_util.hpp>
#include <se/map/utils/type_util.hpp>

namespace se {

// Semantic data
template<Semantics SemB>
struct SemanticData {
    struct Config {
        void readYaml(const std::string& /* yaml_file */)
        {
        }
    };
};

// Semantic data
template<>
struct SemanticData<Semantics::On> {
    semantics_t sem = semantics_t(0);

    struct Config {
        /** Reads the struct members from the "data" node of a YAML file. Members not present in the
         * YAML file aren't modified.
         */
        void readYaml(const std::string& yaml_file);
    };
};

std::ostream& operator<<(std::ostream& os, const SemanticData<Semantics::Off>::Config& c);
std::ostream& operator<<(std::ostream& os, const SemanticData<Semantics::On>::Config& c);

} // namespace se

#endif // SE_DATA_SEMANTICS_HPP
