/*
 * SPDX-FileCopyrightText: 2016-2019 Emanuele Vespa
 * SPDX-FileCopyrightText: 2020-2021 Smart Robotics Lab, Imperial College London, Technical University of Munich
 * SPDX-FileCopyrightText: 2020-2021 Nils Funk
 * SPDX-FileCopyrightText: 2021 Sotiris Papatheodorou
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SE_MARCHING_CUBE_HPP
#define SE_MARCHING_CUBE_HPP

#include <se/common/bounded_vector.hpp>
#include <se/common/timings.hpp>
#include <se/map/algorithms/edge_tables.hpp>
#include <se/map/algorithms/mesh.hpp>
#include <se/map/octree/iterator.hpp>
#include <se/map/octree/visitor.hpp>

namespace se {
namespace meshing {

/// Single-res marching cube implementation

template<typename OctreeT>
Eigen::Vector3f compute_intersection(const OctreeT& octree,
                                     const Eigen::Vector3i& coord_0,
                                     const Eigen::Vector3i& coord_1);

template<typename OctreeT>
Eigen::Vector3f
interp_vertexes(const OctreeT& octree, const int x, const int y, const int z, const int edge);

template<typename BlockT>
void gather_data(const BlockT* block,
                 typename BlockT::DataType data[8],
                 const int x,
                 const int y,
                 const int z);

template<typename OctreeT>
void gather_data(const OctreeT& octree,
                 typename OctreeT::DataType data[8],
                 const int x,
                 const int y,
                 const int z);

template<typename OctreeT>
uint8_t compute_index(const OctreeT& octree,
                      const typename OctreeT::BlockType* block_ptr,
                      const int x,
                      const int y,
                      const int z);



/// Multires-res marching cube implementation

template<typename DataT>
Eigen::Vector3f compute_dual_intersection(const DataT& data_0,
                                          const DataT& data_1,
                                          const Eigen::Vector3f& dual_point_0_M,
                                          const Eigen::Vector3f& dual_point_1_M);

template<typename DataT, typename ValueSelector>
Eigen::Vector3f interp_dual_vertexes(const int edge,
                                     const DataT data[8],
                                     const std::array<Eigen::Vector3f, 8>& dual_corner_coords_f);

/*
 * Normalised offsets of the dual corners from the primal corner
 */
static const Eigen::Vector3f norm_dual_offset_f[8] = {{-1, -1, -1},
                                                      {+1, -1, -1},
                                                      {+1, -1, +1},
                                                      {-1, -1, +1},
                                                      {-1, +1, -1},
                                                      {+1, +1, -1},
                                                      {+1, +1, +1},
                                                      {-1, +1, +1}};

template<typename BlockT, typename DataT>
void gather_dual_data(const BlockT* block,
                      const int scale,
                      const Eigen::Vector3f& primal_corner_coord_f,
                      DataT data[8],
                      std::array<Eigen::Vector3f, 8>& dual_corner_coords_f,
                      std::array<Eigen::Vector3i, 8>& dual_corner_coords_i);

/*! \brief The following strategy is derived from I. Wald, A Simple, General,
 *  and GPU Friendly Method for Computing Dual Mesh and Iso-Surfaces of Adaptive Mesh Refinement (AMR) Data, 2020
 *
 * We validate the scale of all neighbouring blocks need to access the 8 dual corners for each primal corner
 * For each we compute the dual coordinates for primal coordinates with a relative block offset x,y,z in [0, block_size]
 * Due to the fact that block_size is still contained in the offset (rather than [0, block_size - 1], each primal corner
 * is contained in 1 (inside block), 2 (face), 4 (edge) or 8 (corner) neighbouring blocks. We prioritse the block neighbours
 * based on their value (heigher value = higher priority), where the value is calculated via
 * v = 4 << (x is +1) + 2 << (y is +1) + 1 << (z is +1).
 * The minium value is computed for all neighbours for dual corners (c0-8). This means if multiple dual corners fall
 * inside the same neighbouring block we take the minimum value of all the dual corners inside the block.
 * The threshold for populate the lower or higher priority list is the lowest dual corner cost of the corners falling
 * inside the block the relative primal coordinate belongs to.
 *
 * \param[in] primal_corner_coord_rel     relative voxel offset of the primal corner from the block coordinates
 * \param[in] block_size                  size of a voxel block in voxel units
 * \param[in] lower_priority_neighbours   blocks with lower priority, i.e. will be neglected if scale >= scale neighbour
 * \param[in] higher_priority_neighbours  blocks with higher priority, i.e. will only be neglected if scale > scale neighbour
 * \param[in] neighbours                  vector containing a vector with all corner offsets for a neighbouring block.
 *                                        First index is the main block.
 */
inline void norm_dual_corner_idxs(const Eigen::Vector3i& primal_corner_coord_rel,
                                  const int block_size,
                                  BoundedVector<int, 8>& lower_priority_neighbours,
                                  BoundedVector<int, 8>& higher_priority_neighbours,
                                  BoundedVector<BoundedVector<int, 8>, 8>& neighbours);

/*
 * Normalised offsets of the primal corners from the primal corner
 * The correct coordinates would be {-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5}, ...
 * However this would cause a lot of extra computations to cast the voxels back and forth and
 * to make sure that the absolute coordinates {-0.5, y, z} won't be casted to {0, y, z} (and for y, z accordingly).
 */
static const Eigen::Vector3i logical_dual_offset[8] = {{-1, -1, -1},
                                                       {+0, -1, -1},
                                                       {+0, -1, +0},
                                                       {-1, -1, +0},
                                                       {-1, +0, -1},
                                                       {+0, +0, -1},
                                                       {+0, +0, +0},
                                                       {-1, +0, +0}};

template<typename OctreeT, typename DataT>
void gather_dual_data(const OctreeT& octree,
                      const typename OctreeT::BlockType* block,
                      const int scale,
                      const Eigen::Vector3i& primal_corner_coord,
                      DataT data[8],
                      std::array<Eigen::Vector3f, 8>& dual_corner_coords_f,
                      std::array<Eigen::Vector3i, 8>& dual_corner_coords_i);

template<typename OctreeT, typename DataT>
void compute_dual_index(const OctreeT& octree,
                        const typename OctreeT::BlockType* block_ptr,
                        const int scale,
                        const Eigen::Vector3i& primal_corner_coord,
                        uint8_t& edge_pattern_idx,
                        DataT data[8],
                        std::array<Eigen::Vector3f, 8>& dual_corner_coords_f,
                        std::array<Eigen::Vector3i, 8>& dual_corner_coords_i);

inline bool checkVertex(const Eigen::Vector3f& vertex_M, const float dim);

} // namespace meshing



namespace algorithms {

template<typename OctreeT, typename = std::enable_if_t<OctreeT::res_ == se::Res::Single>>
typename OctreeT::SurfaceMesh
marching_cube_kernel(const OctreeT& octree,
                     const std::vector<const typename OctreeT::BlockType*>& block_ptrs);

template<typename OctreeT, typename = std::enable_if_t<OctreeT::res_ == se::Res::Multi>>
typename OctreeT::SurfaceMesh
dual_marching_cube_kernel(const OctreeT& octree,
                          const std::vector<const typename OctreeT::BlockType*>& block_ptrs,
                          const int min_desired_scale);

template<typename OctreeT>
std::vector<meshing::VertexIndexMesh<3>>
dual_marching_cube_kernel_new(const OctreeT& octree,
                              const std::vector<const typename OctreeT::BlockType*>& block_ptrs);

template<typename OctreeT>
meshing::VertexIndexMesh<3>
dual_marching_cube_new(const OctreeT& octree,
                       const std::vector<const typename OctreeT::BlockType*>& block_ptrs);

template<typename OctreeT>
meshing::VertexIndexMesh<3> dual_marching_cube_new(const OctreeT& octree);


/** Return a triangle mesh of the surface in \p octree. For se::Res::Multi octrees a minimum desired
 * scale \p min_desired_scale can be specified in order to obtain a coarser surface mesh. The value
 * of \p min_desired_scale is ignored for se::Res::Single octrees.
 */
template<typename OctreeT>
typename OctreeT::SurfaceMesh marching_cube(const OctreeT& octree, const int min_desired_scale = 0);

} // namespace algorithms
} // namespace se

#include "impl/marching_cube_impl.hpp"

#endif // SE_MARCHING_CUBE_HPP
