/*
 * SPDX-FileCopyrightText: 2016-2019 Emanuele Vespa
 * SPDX-FileCopyrightText: 2019-2023 Smart Robotics Lab, Imperial College London, Technical University of Munich
 * SPDX-FileCopyrightText: 2019-2023 Nils Funk
 * SPDX-FileCopyrightText: 2021-2023 Sotiris Papatheodorou
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SE_VISITOR_HPP
#define SE_VISITOR_HPP

#include <optional>
#include <se/map/octree/allocator.hpp>
#include <se/map/octree/fetcher.hpp>

/**
 * Helper wrapper to traverse the octree. All functions take a const octree references and as no manipulation of the octree is done.
 */
namespace se {
namespace visitor {



/// Single/multi-res get data functions

/**
 * \brief Get the voxel data for a given coordinate.
 *        The function returns init data if the data is not allocated.
 *
 * \tparam OctreeT        The type of the octree used
 * \param[in] octree      The reference to the octree
 * \param[in] voxel_coord The voxel coordinates to be accessed
 *
 * \return The data in the voxel to be accessed
 *         Returns init data if block is not allocated
 */
template<typename OctreeT>
typename OctreeT::DataType getData(const OctreeT& octree, const Eigen::Vector3i& voxel_coord);

/**
 * \brief Get the voxel data for a given coordinate.
 *        The function checks first if the voxel coordinates are contained in the provided block pointer.
 *        If this is not the case the function fetches the correct block.
 *        The function returns init data if the data is not allocated.
 *
 * \tparam OctreeT        The type of the octree used
 * \param[in] octree      The reference to the octree
 * \param[in] block_ptr   The pointer to the block to checked first
 * \param[in] voxel_coord The voxel coordinates to be accessed
 *
 * \return The data in the voxel to be accessed
 *         Returns init data if block is not allocated
 */
template<typename OctreeT, typename BlockT>
typename OctreeT::DataType
getData(const OctreeT& octree, BlockT* block_ptr, const Eigen::Vector3i& voxel_coord);



/// Multi-res get data functions

/**
 * \brief Get the voxel data for a given coordinate and desired scale.
 *        The function returns init data if the data is not allocated.
 *
 * \tparam OctreeT           The type of the octree used
 * \param[in] octree_ptr     The pointer to the octree
 * \param[in] voxel_coord    The voxel coordinates to be accessed
 * \param[in] scale_desired  The scale to fetch the data from (init data for MultiresTSDF at node level)
 * \param[in] scale_returned The scale the data is returned from (max (scale desired, finest scale with valid data)
 *
 * \return The data in octant at the returned scale
 */
template<typename OctreeT>
typename std::enable_if_t<OctreeT::res_ == Res::Multi, typename OctreeT::DataType>
getData(const OctreeT& octree,
        const Eigen::Vector3i& voxel_coord,
        const int scale_desired,
        int& scale_returned);

/**
 * \brief Get the voxel data for a given coordinate and desired scale.
 *        The function checks first if the voxel coordinates are contained in the provided block pointer.
 *        If this is not the case the function fetches the correct block.
 *        The function returns init data if the data is not allocated.
 *
 * \tparam OctreeT           The type of the octree used
 * \param[in] octree         The reference to the octree
 * \param[in] block_ptr      The pointer to the block to checked first
 * \param[in] voxel_coord    The voxel coordinates to be accessed
 * \param[in] scale_desired  The scale to fetch the data from (init data for MultiresTSDF at node level)
 * \param[in] scale_returned The scale the data is returned from (max (scale desired, finest scale with valid data)
 *
 * \return The data in octant at the returned scale
 */
template<typename OctreeT, typename BlockT>
typename std::enable_if_t<OctreeT::res_ == Res::Multi, typename OctreeT::DataType>
getData(const OctreeT& octree,
        BlockT* block_ptr,
        const Eigen::Vector3i& voxel_coord,
        const int scale_desired,
        int& scale_returned);

/**
 * \brief Get the min occupancy data at a given scale.
 *
 * \tparam OctreeT      The type of octree used (has to be of field type occupancy and multi-res)
 * \param octree        The reference to the octree
 * \param voxel_coord   The voxel coordinates in [voxel] to be accessed
 * \param scale_desired The scale to be accessed
 * \return The min data at the requested scale.
 */
template<typename OctreeT>
inline typename std::enable_if_t<OctreeT::DataType::fld_ == se::Field::Occupancy,
                                 typename OctreeT::DataType>
getMinData(const OctreeT& octree, const Eigen::Vector3i& voxel_coord, const int scale_desired);

/**
 * \brief Get the max occupancy data at a given scale.
 *
 * \tparam OctreeT      The type of octree used (has to be of field type occupancy and multi-res)
 * \param octree        The reference to the octree
 * \param voxel_coord   The voxel coordinates in [voxel] to be accessed
 * \param scale_desired The scale to be accessed
 * \return The max data at the requested scale.
 */
template<typename OctreeT>
typename std::enable_if_t<OctreeT::DataType::fld_ == Field::Occupancy, typename OctreeT::DataType>
getMaxData(const OctreeT& octree, const Eigen::Vector3i& voxel_coord, const int scale_desired);

// TODO: Reduce getField functions for single and multi-res to one

/// Single/Multi-res get field functions

/**
 * \brief Get the field value for a given coordinate.
 *        The function returns {}/invalid if the data is invalid.
 *
 * \tparam OctreeT        The type of the octree used
 * \param[in] octree      The reference to the octree
 * \param[in] voxel_coord The voxel coordinates to be accessed
 *
 * \return The field value to be accessed if the data is valid, {}/invalid otherwise
 */
template<typename OctreeT>
std::optional<field_t> getField(const OctreeT& octree, const Eigen::Vector3i& voxel_coord);

/**
 * \brief Get the field value for a given coordinate.
 *        The function returns {}/invalid if the data is invalid.
 *        The function checks first if the voxel coordinates are contained in the provided block pointer.
 *        If this is not the case the function fetches the correct octant.
 *
 * \tparam OctreeT        The type of the octree used
 * \param[in] octree      The reference to the octree
 * \param[in] block_ptr   The pointer to the block to checked first
 * \param[in] voxel_coord The voxel coordinates to be accessed
 *
 * \return The field value to be accessed if the data is valid, {}/invalid otherwise
 */
template<typename OctreeT, typename BlockT>
std::optional<field_t>
getField(const OctreeT& octree, BlockT* block_ptr, const Eigen::Vector3i& voxel_coord);



/// Multi-res get field functions

/**
 * \brief Get the field value for a given coordinate and desired scale.
 *        The function returns {}/invalid if the data is invalid.
 *
 * \tparam OctreeT           The type of the octree used
 * \param[in] octree         The reference to the octree
 * \param[in] voxel_coord    The voxel coordinates to be accessed
 * \param[in] scale_desired  The scale to fetch the data from (init data for MultiresTSDF at node level)
 * \param[in] scale_returned The scale the field value is returned from (max (scale desired, finest scale with valid data)
 *
 * \return The field value at the returned scale if the data is valid, {}/invalid otherwise
 */
template<typename OctreeT>
typename std::enable_if_t<OctreeT::res_ == Res::Multi, std::optional<field_t>>
getField(const OctreeT& octree,
         const Eigen::Vector3i& voxel_coord,
         const int scale_desired,
         int& scale_returned);

/**
 * \brief Get the field value for a given coordinate and desired scale.
 *        The function returns {}/invalid if the data is invalid.
 *        The function checks first if the voxel coordinates are contained in the provided block pointer.
 *        If this is not the case the function fetches the correct octant.
 *
 * \tparam OctreeT           The type of the octree used
 * \param[in] octree         The reference to the octree
 * \param[in] block_ptr      The pointer to the block to checked first
 * \param[in] voxel_coord    The voxel coordinates to be accessed
 * \param[in] scale_desired  The scale to fetch the data from (init data for MultiresTSDF at node level)
 * \param[in] scale_returned The scale the field value is returned from (max (scale desired, finest scale with valid data)
 *
 * \return The field value at the returned scale if the data is valid, {}/invalid otherwise
 */
template<typename OctreeT, typename BlockT>
typename std::enable_if_t<OctreeT::res_ == Res::Multi, std::optional<field_t>>
getField(const OctreeT& octree,
         BlockT* block_ptr,
         const Eigen::Vector3i& voxel_coord,
         const int scale_desired,
         int& scale_returned);



/** Interpolate a member of se::Octree::DataType at the supplied voxel coordinates and desired
 * scale. The scale the member is interpolated at may be coarser than \p desired_scale and is
 * written in \p returned_scale.
 *
 * \param[in] octree          The multi-resolution octree containing the data.
 * \param[in] voxel_coord_f   The voxel coordinates the member will be interpolated at. The
 *                            coordinates may have a fractional part.
 * \param[in] valid           A functor with the following prototype, returning whether the supplied
 *                            data is valid and should be used for interpolation:
 *                            \code{.cpp}
 *                            template<typename OctreeT>
 *                            bool valid(const typename OctreeT::DataType& data);
 *                            \endcode
 * \param[in] get             A functor with the following prototype, returning the member of type
 *                            `T` to be interpolated:
 *                            \code{.cpp}
 *                            template<typename OctreeT>
 *                            T get(const typename OctreeT::DataType& data);
 *                            \endcode
 *                            Type `T` must implement the following operators:
 *                            \code{.cpp}
 *                            T operator+(const T& a, const T& b);
 *                            T operator*(const T& a, const float b);
 *                            \endcode
 * \param[in]  desired_scale  The finest scale the member should be interpolated at.
 * \param[out] returned_scale The actual scale the member was interpolated at will be stored into
 *                            `*returned_scale` if `returned_scale` is non-null. `*returned_scale`
 *                            is not modified if `std::nullopt` is returned. The value of
 *                            `*returned_scale` will not be less than \p desired_scale.
 * \return The interpolated member if the data is valid, `std::nullopt` otherwise.
 */
template<typename OctreeT, typename ValidF, typename GetF>
typename std::enable_if_t<OctreeT::res_ == Res::Multi,
                          std::optional<std::invoke_result_t<GetF, typename OctreeT::DataType>>>
getInterp(const OctreeT& octree,
          const Eigen::Vector3f& voxel_coord_f,
          ValidF valid,
          GetF get,
          const int desired_scale = 0,
          int* const returned_scale = nullptr);

/** \overload
 * \details This overload works only for single-resolution octrees. The member is interpolated at
 * scale 0, the finest and only scale.
 */
template<typename OctreeT, typename ValidF, typename GetF>
typename std::enable_if_t<OctreeT::res_ == Res::Single,
                          std::optional<std::invoke_result_t<GetF, typename OctreeT::DataType>>>
getInterp(const OctreeT& octree, const Eigen::Vector3f& voxel_coord_f, ValidF valid, GetF get);



/** Interpolate the field at the supplied voxel coordinates and desired scale. The scale the field
 * is interpolated at may be coarser than \p desired_scale and is written in \p returned_scale.
 *
 * \param[in] octree          The multi-resolution octree containing the data.
 * \param[in] voxel_coord_f   The voxel coordinates the field will be interpolated at. The
 *                            coordinates may have a fractional part.
 * \param[in]  desired_scale  The finest scale the field should be interpolated at.
 * \param[out] returned_scale The actual scale the field was interpolated at will be stored into
 *                            `*returned_scale` if `returned_scale` is non-null. `*returned_scale`
 *                            is not modified if `std::nullopt` is returned. The value of
 *                            `*returned_scale` will not be less than \p desired_scale.
 * \return The interpolated field value if the data is valid, `std::nullopt` otherwise.
 */
template<typename OctreeT>
typename std::enable_if_t<OctreeT::res_ == Res::Multi, std::optional<field_t>>
getFieldInterp(const OctreeT& octree,
               const Eigen::Vector3f& voxel_coord_f,
               const int desired_scale = 0,
               int* const returned_scale = nullptr);

/** \overload
 * \details This overload works only for single-resolution octrees. The field is interpolated at
 * scale 0, the finest and only scale.
 */
template<typename OctreeT>
typename std::enable_if_t<OctreeT::res_ == Res::Single, std::optional<field_t>>
getFieldInterp(const OctreeT& octree, const Eigen::Vector3f& voxel_coord_f);

/** Interpolate the colour at the supplied voxel coordinates and desired scale. The scale the colour
 * is interpolated at may be coarser than \p desired_scale and is written in \p returned_scale.
 *
 * \param[in] octree          The multi-resolution octree containing the data.
 * \param[in] voxel_coord_f   The voxel coordinates the colour will be interpolated at. The
 *                            coordinates may have a fractional part.
 * \param[in]  desired_scale  The finest scale the colour should be interpolated at.
 * \param[out] returned_scale The actual scale the colour was interpolated at will be stored into
 *                            `*returned_scale` if `returned_scale` is non-null. `*returned_scale`
 *                            is not modified if `std::nullopt` is returned. The value of
 *                            `*returned_scale` will not be less than \p desired_scale.
 * \return The interpolated colour if the data is valid, `std::nullopt` otherwise.
 */
template<typename OctreeT>
typename std::enable_if_t<OctreeT::res_ == Res::Multi && OctreeT::col_ == Colour::On,
                          std::optional<colour_t>>
getColourInterp(const OctreeT& octree,
                const Eigen::Vector3f& voxel_coord_f,
                const int desired_scale = 0,
                int* const returned_scale = nullptr);

/** \overload
 * \details This overload works only for single-resolution octrees. The colour is interpolated at
 * scale 0, the finest and only scale.
 */
template<typename OctreeT>
typename std::enable_if_t<OctreeT::res_ == Res::Single && OctreeT::col_ == Colour::On,
                          std::optional<colour_t>>
getColourInterp(const OctreeT& octree, const Eigen::Vector3f& voxel_coord_f);



/// Single-res get gradient functions

/**
 * \brief Get the field gradient for a given coordinate [float voxel coordinates].
 *        The function returns {}/invalid if the gradient is invalid.
 *
 * \tparam OctreeT          The type of the octree used
 * \param[in] octree        The reference to the octree
 * \param[in] voxel_coord_f The voxel coordinates to be accessed [float voxel coordiantes]
 *
 * \return The field gradient if the gradient is valid, {}/invalid otherwise
 */
template<typename OctreeT>
typename std::enable_if_t<OctreeT::res_ == Res::Single, std::optional<field_vec_t>>
getFieldGrad(const OctreeT& octree, const Eigen::Vector3f& voxel_coord_f);



/// Multi-res get gradient functions

/**
 * \brief Get the field gradient for a given coordinate [float voxel coordinates].
 *        The function returns {}/invalid if the gradient is invalid.
 *
 * \tparam OctreeT          The type of the octree used
 * \param[in] octree        The reference to the octree
 * \param[in] voxel_coord_f The voxel coordinates to be accessed [float voxel coordiantes]
 *
 * \return The field gradient if the gradient is valid, {}/invalid otherwise
 */
template<typename OctreeT>
typename std::enable_if_t<OctreeT::res_ == Res::Multi, std::optional<field_vec_t>>
getFieldGrad(const OctreeT& octree, const Eigen::Vector3f& voxel_coord_f);

/**
 * \brief Get the field gradient for a given coordinate [float voxel coordinates].
 *        The function returns {}/invalid if the gradient is invalid.
 *
 * \tparam OctreeT           The type of the octree used
 * \param[in] octree         The reference to the octree
 * \param[in] voxel_coord_f  The voxel coordinates to be accessed [float voxel coordiantes]
 * \param[in] scale_returned The scale the gradient has been computed at
 *
 * \return The field gradient if the gradient is valid, {}/invalid otherwise
 */
template<typename OctreeT>
typename std::enable_if_t<OctreeT::res_ == Res::Multi, std::optional<field_vec_t>>
getFieldGrad(const OctreeT& octree, const Eigen::Vector3f& voxel_coord_f, int& scale_returned);

/**
 * \brief Get the field gradient for a given coordinate [float voxel coordinates] and desired scale.
 *        The function returns {}/invalid if the gradient is invalid.
 *
 * \tparam OctreeT           The type of the octree used
 * \param[in] octree         The reference to the octree
 * \param[in] voxel_coord_f  The voxel coordinates to be accessed [float voxel coordiantes]
 * \param[in] scale_desired  The finest scale to compute the gradient at
 * \param[in] scale_returned The scale the gradient has been computed at (max (scale desired, finest common neighbour scale)
 *
 * \return The field gradient if the gradient is valid, {}/invalid otherwise
 */
template<typename OctreeT>
typename std::enable_if_t<OctreeT::res_ == Res::Multi, std::optional<field_vec_t>>
getFieldGrad(const OctreeT& octree,
             const Eigen::Vector3f& voxel_coord_f,
             const int scale_desired,
             int& scale_returned);

} // namespace visitor
} // namespace se

#include "impl/visitor_impl.hpp"

#endif // SE_VISITOR_HPP
