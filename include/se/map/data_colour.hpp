/*
 * SPDX-FileCopyrightText: 2021-2022 Smart Robotics Lab, Imperial College London, Technical University of Munich
 * SPDX-FileCopyrightText: 2021-2022 Nils Funk
 * SPDX-FileCopyrightText: 2021-2022 Sotiris Papatheodorou
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SE_DATA_COLOUR_HPP
#define SE_DATA_COLOUR_HPP

#include "utils/setup_util.hpp"
#include "utils/type_util.hpp"

namespace se {

// Defaults
static constexpr rgba_t dflt_rgba = 0xFFFFFFFF; // White
static constexpr rgba_t dflt_delta_rgba = 0;

// Colour data
template<Colour ColB>
struct ColourData {
    struct Config {
        Config()
        {
        }
        Config(const std::string& /* yaml_file */)
        {
        }
    };
};

template<Colour ColB>
std::ostream& operator<<(std::ostream& os, const typename ColourData<ColB>::Config& /* c */)
{
    return os;
}

template<>
struct ColourData<Colour::On> {
    struct Config {
        /** Initializes the config to some sensible defaults.
         */
        Config();

        /** Initializes the config from a YAML file. Data not present in the YAML file will be
         * initialized as in ColourData<se::Colour::On>::Config().
         */
        Config(const std::string& yaml_file);
    };

    rgba_t rgba = dflt_rgba;
};

template<>
std::ostream& operator<< <Colour::On>(std::ostream& os, const ColourData<Colour::On>::Config& c);

///////////////////
/// DELTA DATA  ///
///////////////////

// Colour data
template<Colour ColB>
struct ColourDeltaData {};

template<>
struct ColourDeltaData<Colour::On> {
    rgba_t delta_rgba = dflt_delta_rgba;
};

} // namespace se

#endif // SE_DATA_COLOUR_HPP
