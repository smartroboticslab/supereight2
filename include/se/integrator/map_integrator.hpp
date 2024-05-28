/*
 * SPDX-FileCopyrightText: 2021 Smart Robotics Lab, Imperial College London, Technical University of Munich
 * SPDX-FileCopyrightText: 2021 Nils Funk
 * SPDX-FileCopyrightText: 2021 Sotiris Papatheodorou
 * SPDX-FileCopyrightText: 2022-2024 Simon Boche
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SE_MAP_INTEGRATOR_HPP
#define SE_MAP_INTEGRATOR_HPP

#include <cstddef>
#include <iterator>

#include "measurement.hpp"
#include "se/common/math_util.hpp"
#include "se/integrator/allocator/raycast_carver.hpp"
#include "se/integrator/allocator/volume_carver.hpp"
#include "se/integrator/ray_integrator.hpp"
#include "se/map/octree/fetcher.hpp"
#include "se/map/octree/integrator.hpp"
#include "se/map/utils/setup_util.hpp"

namespace se {
namespace allocator {

/**
 * \brief Allocate frustum in band around the surface.
 *
 * \tparam MapT
 * \tparam SensorT
 * \param[in] map       The reference to the map
 * \param[in] sensor    The sensor use for the projection
 * \param[in] depth_img The sensor depth image
 * \param[in] T_WS      The transformation from sensor to world frame
 * \param[in] band      The size of the band allocated around the surface
 *
 * \return The allocated and fetched notes in the band around the surface measurements
 */
template<typename MapT, typename SensorT>
std::vector<se::OctantBase*> frustum(MapT& map,
                                     SensorT& sensor,
                                     const se::Image<float>& depth_img,
                                     const Eigen::Isometry3f& T_WS,
                                     const float band);

} // namespace allocator



namespace fetcher {
/**
 * \brief Return the currently allocated Blocks that intersect the camera frustum.
 * Some false positives might be returned since Blocks are approximated by their bounding spheres
 * and because sphereInFrustum() may return false positives in rare cases.
 *
 * \tparam MapT    The map type.
 * \tparam SensorT The sensor type.
 * \param map      The map to fetch Blocks from.
 * \param sensor   The sensor whose frustum is used for the test.
 * \param T_WS     The pose of the sensor in the world frame.
 *
 * \return A vector of pointers to Blocks that intersect the sensor frustum.
 */
template<typename MapT, typename SensorT>
inline std::vector<se::OctantBase*>
frustum(MapT& map, const SensorT& sensor, const Eigen::Isometry3f& T_WS);
} // namespace fetcher



template<typename MapT>
class MapIntegrator {
    public:
    MapIntegrator(MapT& map);

    /** Integrate the images in \p measurements captured at \p timestamp into the map. If \p
     * updated_octants is non-null, *updated_octants will contain pointers to the octants updated
     * during this integration.
     *
     * \warning The caller must ensure that any in \p measurements remains valid until
     * se::MapIntegrator::integrateDepth() returns.
     */
    template<typename SensorT>
    void integrateDepth(const Measurements<SensorT>& measurements,
                        const timestamp_t timestamp,
                        std::vector<const OctantBase*>* updated_octants = nullptr);

    /**
     * \brief Integrate depth image into the maps field representation.
     *
     * \tparam SensorT
     * \param[in] depth_img       The sensor depth image
     * \param[in] sensor          The sensor use for the projection
     * \param[in] T_WS            The transformation from sensor to world frame
     * \param[in] timestamp       The timestamp of the image to be integrated
     * \param[in] updated_octants Pointers to the octants updates during integration will be stored
     *                            in \p updated_octants if it's not \p nullptr.
     */
    template<typename SensorT>
    void integrateDepth(const SensorT& sensor,
                        const se::Image<float>& depth_img,
                        const Eigen::Isometry3f& T_WS,
                        const timestamp_t timestamp,
                        std::vector<const OctantBase*>* updated_octants = nullptr);

    template<typename SensorT>
    void integrateDepth(const SensorT& sensor,
                        const se::Image<float>& depth_img,
                        const Eigen::Isometry3f& T_WS,
                        const SensorT& colour_sensor,
                        const se::Image<colour_t>& colour_img,
                        const Eigen::Isometry3f& T_SSc,
                        const timestamp_t timestamp,
                        std::vector<const OctantBase*>* updated_octants = nullptr);

    /**
     * \brief Integrate single ray measurement into the maps field representation.
     *
     * \tparam SensorT
     * \param[in] ray_S           The measured ray in sensor frame
     * \param[in] sensor          The sensor use for the projection
     * \param[in] T_WS            The transformation from sensor to world frame
     * \param[in] timestamp       The timestamp of the ray to be integrated
     * \param[in] updated_octants Pointers to the octants updates during integration will be stored
     *                            in \p updated_octants if it's not \p nullptr.
     */
    template<typename SensorT>
    void integrateRay(const SensorT& sensor,
                      const Eigen::Vector3f& ray_S,
                      const Eigen::Isometry3f& T_WS,
                      const timestamp_t timestamp,
                      std::vector<const OctantBase*>* updated_octants = nullptr);



    /**
     * \brief Integrate a batch of ray images into the maps field representation.
     *
     * \tparam SensorT
     * \param[in] sensor          The sensor use for the projection
     * \param[in] rayBatch        The batch of ray measurements (in sensor frame)
     * \param[in] poseBatch       The batch of corresponding sensor poses in the world frame
     * \param[in] timestamp       The timestamp of the batch to be integrated
     * \param[in] updated_octants Pointers to the octants updates during integration will be stored
     *                            in \p updated_octants if it's not \p nullptr.
     */
    template<typename SensorT>
    void integrateRayBatch(
        const SensorT& sensor,
        const std::vector<std::pair<Eigen::Isometry3f, Eigen::Vector3f>,
                          Eigen::aligned_allocator<std::pair<Eigen::Isometry3f, Eigen::Vector3f>>>&
            rayPoseBatch,
        const timestamp_t timestamp,
        std::vector<const OctantBase*>* updated_octants = nullptr);

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    private:
    MapT& map_;
};



} // namespace se

#include "impl/map_integrator_impl.hpp"

#endif // SE_MAP_INTEGRATOR_HPP
