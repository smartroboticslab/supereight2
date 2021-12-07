/*
 * SPDX-FileCopyrightText: 2020-2021 Smart Robotics Lab, Imperial College London, Technical University of Munich
 * SPDX-FileCopyrightText: 2020 Marija Popovic
 * SPDX-FileCopyrightText: 2020-2021 Nils Funk
 * SPDX-FileCopyrightText: 2020-2021 Sotiris Papatheodorou
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __READER_NEWERCOLLEGE_HPP
#define __READER_NEWERCOLLEGE_HPP

#include <Eigen/Dense>
#include <cstdint>
#include <fstream>
#include <string>

#include "reader_base.hpp"
#include "se/common/projection.hpp"
#include "se/image/image.hpp"



namespace se {

/** Reader for Newer College datasets. */
class NewerCollegeReader : public Reader {
    public:
    /** Construct an NewerCollegeReader from a ReaderConfig.
     *
     * \param[in] c The configuration struct to use.
     */
    NewerCollegeReader(const ReaderConfig& c);

    /** Restart reading from the beginning. */
    void restart();

    /** The name of the reader.
     *
     * \return The string `"NewerCollegeReader"`.
     */
    std::string name() const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    private:

    ReaderStatus nextDepth(Image<float>& depth_image);

    ReaderStatus nextRGBA(Image<uint32_t>& rgba_image);

    static constexpr int8_t pixel_offset[64] = {
        0,  6,  12, 18, 0,  6,  12, 18, 0,  6,  12, 18, 0,  6,  12, 18, 0,  6,  12, 18, 0,  6,
        12, 18, 0,  6,  12, 18, 0,  6,  12, 18, 0,  6,  12, 18, 0,  6,  12, 18, 0,  6,  12, 18,
        0,  6,  12, 18, 0,  6,  12, 18, 0,  6,  12, 18, 0,  6,  12, 18, 0,  6,  12, 18};

    /** Return the number of LIDAR scans in the supplied directory.
     * LIDAR scans are considered those whose name conforms to the pattern
     * cloud_XXXX.pcd where X is a digit 0-9.
     *
     * \param[in] dir The directory inside which to look for depth images.
     * \return The number of LIDAR scans found.
     */
    size_t numScans(const std::string& dir) const;
};

} // namespace se

#endif
