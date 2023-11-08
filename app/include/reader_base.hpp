/*
 * SPDX-FileCopyrightText: 2014 University of Edinburgh, Imperial College London, University of Manchester
 * SPDX-FileCopyrightText: 2016-2019 Emanuele Vespa
 * SPDX-FileCopyrightText: 2020-2023 Smart Robotics Lab, Imperial College London, Technical University of Munich
 * SPDX-FileCopyrightText: 2020-2022 Nils Funk
 * SPDX-FileCopyrightText: 2020-2023 Sotiris Papatheodorou
 * SPDX-FileCopyrightText: 2022-2024 Simon Boche
 * SPDX-License-Identifier: MIT
 */

#ifndef __READER_BASE_HPP
#define __READER_BASE_HPP

#include <Eigen/Geometry>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <string>

#include "se/common/str_utils.hpp"
#include "se/image/image.hpp"



namespace se {

enum class ReaderType {
    /** Use the se::OpenNIReader. */
    OPENNI,
    /** Use the se::RAWReader. */
    RAW,
    /** Use the se::TUMReader. */
    TUM,
    /** Use the se::InteriorNetReader. */
    INTERIORNET,
    /** Use the se::NewerCollegeReader. */
    NEWERCOLLEGE,
    /** Use the se::LeicaReader. */
    LEICA,
    UNKNOWN
};

ReaderType string_to_reader_type(const std::string& s);

std::string reader_type_to_string(ReaderType t);


/** The result of trying to read a depth/RGB image or a pose.
 */
enum class ReaderStatus : int {
    /** Data read successfully. */
    ok = 0,
    /** Temporary data read error.
     * Further reads might succeed. Typically used to indicate an invalid
     * image or pose. */
    skip,
    /** End of dataset reached. */
    eof,
    /** Fatal data read error.
     * No further read should be attempted. Typically used to indicate that
     * the dataset could not be read at all or no camera was found. */
    error,
};

std::ostream& operator<<(std::ostream& os, const ReaderStatus& s);



/** Base abstract class for dataset readers.
 *
 * The overloaded nextData() functions all increment the frame counter, so
 * only one of them should be called within a single pipeline iteration.
 *
 * \note Derived classes should adhere to the following rules:
 *     - The se::Reader constructor should be called at the very beginning of
 *       all derived class constructors.
 *     - se::Reader::restart() should be called at the very beginning of all
 *       derived class implementations of restart().
 *     - The value of se::Reader::is_live_reader_ should be set appropriately
 *       at the derived class constructor.
 *     - The value of se::Reader::status_ should only be set in derived class
 *       constructors and se::Reader::restart() if applicable.
 */
class Reader {
    public:
    struct Config {
        /** The type of the dataset reader to use.
         */
        ReaderType reader_type = se::ReaderType::RAW;

        /** The path to the dataset. This might be a path to a file or a directory depending on the
         * reader type.
         */
        std::string sequence_path;

        /** The path to the ground truth file.
         */
        std::string ground_truth_file;

        /** The scaling factor to convert depth values to metres. A value of 0 will use the default
         * scaling for the particular dataset. This only needs to be set when using a modified dataset,
         * e.g. when using a dataset in the TUM format with depth scaled by 1000 instead of the default
         * 5000 for TUM, inverse_scale would need to be set to 1/1000.
         */
        float inverse_scale = 0.0f;

        /** The rate in Hz at which dataset frames are read. A value of 0 will result in reading frames
         * as quickly as possible. If frames can be processed faster than they are read, the program
         * will wait before reading the next frame until the target rate is reached. If frames are
         * processed slower than they are read the behaviour depends on the value of
         * se::Reader::Config::drop_frames.
         */
        float fps = 0.0f;

        /** Whether to drop frames when they can't be processed fast enough. This option only has an
         * effect if se::Reader::Config::fps is greater than 0 and frames are read faster than they can be
         * processed:
         * - When false, all frames in the dataset will be processed. This will result in a processing
         * rate lower than what was specified in se::Reader::Config::fps. This simulates a camera with a
         * framerate of se::Reader::Config::fps and processing every frame produced by the camera.
         * - When true, input frames will be skipped, assumming they arrive at a rate of
         * se::Reader::Config::fps. This simulates a camera with a framerate of se::Reader::Config::fps and
         * always processing the laster frame produced by the camera.
         */
        bool drop_frames = false;

        /** The verbosity level of dataset readers. A positive value results in more information being
         * printed to standard output when reading data. Greater values result in more information being
         * printed.
         */
        int verbose = 0;

        /** Provide Transformation Sensor (LiDAR) to Body (Drone) also to reader. T_BS.
         *  \note Only for se::ReaderType::LEICA ("rangeImage" or "ray")
         */
        Eigen::Isometry3f T_BL = Eigen::Isometry3f::Identity();

        /** The time for the interval of LiDAR Measurements that are grouped together as one scan and
         *  converted to one range image
         *  \note Only for se::ReaderType::LEICA ("rangeImage" or "ray")
         */
        float scan_time_interval = 1.0;

        /** Reads the struct members from the "reader" node of a YAML file. Members not present in the
         * YAML file aren't modified.
         */
        void readYaml(const std::string& filename);
    };


    /** Construct a Reader from a Config.
     *
     * \note This constructor should be called in all derived class
     *       constructors.
     *
     * \param[in] c The configuration struct to use.
     */
    Reader(const Config& c);

    virtual ~Reader(){};

    /** Read the next depth image.
     *
     * \note The frame number is incremented when calling this function.
     *
     * \param[out] depth_image The next depth image.
     * \return An appropriate status code.
     */
    ReaderStatus nextData(Image<float>& depth_image);

    /** Read the next depth and RGBA images.
     *
     * \note The frame number is incremented when calling this function.
     *
     * \param[out] depth_image The next depth image.
     * \param[out] rgba_image  The next RGBA image.
     * \return An appropriate status code.
     */
    ReaderStatus nextData(Image<float>& depth_image, Image<uint32_t>& rgba_image);


    /** Read the next ray and ground truth pose.
    *
    * \note The frame number is incremented when calling this function.
    *
    * \param[out] ray_measurement The next ray (LiDAR) measurement.
    * \param[out] T_WB        The next ground truth pose.
    * \return An appropriate status code.
    */
    ReaderStatus nextData(Eigen::Vector3f& ray_measurement, Eigen::Isometry3f& T_WB);

    /** Read the next batch of rays and ground truth poses.
    *
    * \note The frame number is incremented when calling this function.
    *
    * \param[in]  batch_interval Time interval over which measurements are aggregated
    * \param[out] rayPoseBatch   The next batch of ray (LiDAR) measurements and corresponding poses.
    * \return An appropriate status code.
    */
    ReaderStatus
    nextData(const float batch_interval,
             std::vector<std::pair<Eigen::Matrix4f, Eigen::Vector3f>,
                         Eigen::aligned_allocator<std::pair<Eigen::Matrix4f, Eigen::Vector3f>>>&
                 rayPoseBatch);

    /** Read the next depth and RGBA images and ground truth pose.
     *
     * \note The frame number is incremented when calling this function.
     *
     * \param[out] depth_image The next depth image.
     * \param[out] rgba_image  The next RGBA image.
     * \param[out] T_WB        The next ground truth pose.
     * \return An appropriate status code.
     */
    ReaderStatus
    nextData(Image<float>& depth_image, Image<uint32_t>& rgba_image, Eigen::Isometry3f& T_WB);

    /** Read the ground truth pose at the provided frame number.
     * Each line in the ground truth file should correspond to a single
     * depth/RGBA image pair and have a format<br>
     * `... tx ty tz qx qy qz qw`,<br>
     * that is the pose is encoded in the last 7 columns of the line.
     *
     * \param[in]  frame The frame number of the requested ground truth pose.
     * \param[out] T_WB  The ground truth pose.
     * \return An appropriate status code.
     */
    ReaderStatus getPose(Eigen::Isometry3f& T_WB, const size_t frame);

    /** Restart reading from the beginning.
     *
     * \note Although this is a virtual function, it does have a default
     *       implementation that resets the Reader protected variables.
     *       Reader::restart() (this default implementation) should be called
     *       at the very beginning of any implementation of restart().
     */
    virtual void restart() = 0;

    /** The name of the reader.
     *
     * \return The name of the reader as a string.
     */
    virtual std::string name() const = 0;

    /** The state of the reader.
     * If good() returns false, there is no need to keep reading frames.
     *
     * \return True if the last call to one of the Reader::next*() functions
     *         completed successfully.
     */
    bool good() const;

    /** The current frame number.
     * Frame numbering starts from 0, so the number of frames read so far is
     * `frame() + 1`.
     *
     * \return The number of frames read. Returns SIZE_MAX if no frames
     *         have been read yet.
     */
    size_t frame() const;

    /** The total number of frames in the current dataset.
     *
     * \return The total number of frames. Returns 0 if the number of frames
     *         is unknown (e.g. for camera input).
     */
    size_t numFrames() const;

    /** The dimensions of the depth images.
     *
     * \return A 2D vector containing the width and height of the images.
     */
    Eigen::Vector2i depthImageRes() const;

    /** The dimensions of the RGBA images.
     *
     * \return A 2D vector containing the width and height of the images.
     */
    Eigen::Vector2i RGBAImageRes() const;

    /** Whether the reader uses a live camera as input.
     *
     * \return True if the input is from a live camera, false otherwise.
     */
    bool isLiveReader() const;

    /** Merge se::ReaderStatus values keeping the worst one.
     *
     * \param[in] status_1 The first se::ReaderStatus to merge.
     * \param[in] status_2 The second se::ReaderStatus to merge.
     * \return The worst of the 2 statuses.
     */
    static ReaderStatus mergeStatus(ReaderStatus status_1, ReaderStatus status_2);

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    protected:
    std::string sequence_path_;
    std::string ground_truth_file_;
    std::ifstream ground_truth_fs_;
    Eigen::Vector2i depth_image_res_;
    Eigen::Vector2i rgba_image_res_;
    float fps_;
    double spf_;
    bool drop_frames_;
    int verbose_;
    bool is_live_reader_;
    ReaderStatus status_;
    /** The frame_ is initialized to SIZE_MAX, so that when first incremented
     * it becomes 0. Unsigned integer overflow is defined behaviour in C/C++
     * so this is safe to do.
     */
    size_t frame_;
    size_t num_frames_;


    /** Read the ground truth pose at the provided frame number.
     * Each line in the ground truth file should correspond to a single
     * depth/RGBA image pair and have a format<br>
     * `... tx ty tz qx qy qz qw`,<br>
     * that is the pose is encoded in the last 7 columns of the line.
     *
     * \note Use getPose(...) to request a pose. It keeps track of the ground_truth_frame_
     *       and ifstream state.
     *
     * \param[out] T_WB      The ground truth pose.
     * \param[in]  frame     The frame number of the requested ground truth pose.
     * \param[in]  delimiter The character delimiting columns in the file. Defaults to space.
     * \return An appropriate status code.
     */
    ReaderStatus readPose(Eigen::Isometry3f& T_WB, const size_t frame, const char delimiter = ' ');


    /** Read the next ground truth pose.
     * Each line in the ground truth file should correspond to a single
     * depth/RGBA image pair and have a format<br>
     * `... tx ty tz qx qy qz qw`,<br>
     * that is the pose is encoded in the last 7 columns of the line.
     *
     * \note The frame number is NOT incremented inside this function.
     *
     * \param[out] T_WB The next ground truth pose.
     * \return An appropriate status code.
     */
    ReaderStatus nextPose(Eigen::Isometry3f& T_WB);

    private:
    size_t ground_truth_frame_;
    char ground_truth_delimiter_;
    std::chrono::steady_clock::time_point prev_frame_timestamp_;

    /** Prepare for reading the next frame.
     * If Reader::fps_ is 0, this function just increments the frame number
     * by 1. Otherwise it sleeps for the required time so that the desired
     * framerate is achieved. It then increments the frame number by 1 or
     * more, so that the value of Reader::drop_frames_ is respected.
     */
    void nextFrame();

    /** Read next ray measurement.
     *
     * \param[out] ray_measurement The next (lidar) ray measurement.
     * \return An appropriate status code.
     */
    virtual ReaderStatus nextRay(Eigen::Vector3f& ray_measurement);

    /** Read next batch of ray measurements.
     *
     * \param[in]  batch_interval (Time) Interval over which measurements are aggregated
     * \param[out] rayPoseBatch   The batch of (lidar) ray measurements.
     * \return An appropriate status code.
     */
    virtual ReaderStatus
    nextRayBatch(const float batch_interval,
                 std::vector<std::pair<Eigen::Matrix4f, Eigen::Vector3f>,
                             Eigen::aligned_allocator<std::pair<Eigen::Matrix4f, Eigen::Vector3f>>>&
                     rayPoseBatch);
    /** Read the next depth image.
     *
     * \param[out] depth_image The next depth image.
     * \return An appropriate status code.
     */
    virtual ReaderStatus nextDepth(Image<float>& depth_image) = 0;

    /** Read the next RGBA image.
     *
     * \param[out] rgba_image The next RGBA image.
     * \return An appropriate status code.
     */
    virtual ReaderStatus nextRGBA(Image<uint32_t>& rgba_image) = 0;
};

std::ostream& operator<<(std::ostream& os, const Reader::Config& c);

} // namespace se

/** The overview of the reader configuration in configuration format.
 *
 * \return The configuration of the reader to the ostream.
 */
static inline std::ostream& operator<<(std::ostream& out, se::Reader* reader)
{
    out << se::str_utils::header_to_pretty_str("READER") << "\n";
    out << se::str_utils::str_to_pretty_str(reader->name(), "Reader type") << "\n";
    out << se::str_utils::str_to_pretty_str(
        ((reader->numFrames() == 0) ? "Unknown" : std::to_string(reader->numFrames())),
        "Number frames")
        << "\n";
    out << "\n";
    return out;
}


#endif
