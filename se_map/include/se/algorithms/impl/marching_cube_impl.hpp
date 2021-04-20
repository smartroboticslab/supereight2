#ifndef SE_MARCHING_CUBE_IMPL_HPP
#define SE_MARCHING_CUBE_IMPL_HPP

#include "se/fetcher.hpp"
#include "se/iterator.hpp"
#include "se/visitor.hpp"

namespace se {

namespace meshing {



/// Single-res marching cube implementation

template <typename OctreeT>
inline Eigen::Vector3f compute_intersection(const std::shared_ptr<OctreeT> octree_ptr,
                                            const Eigen::Vector3i&         source_coord,
                                            const Eigen::Vector3i&         dest_coord)
{

  typename OctreeT::DataType data_0;
  se::visitor::getData(octree_ptr, source_coord, data_0);
  float value_0 = get_field(data_0);

  typename OctreeT::DataType data_1;

  se::visitor::getData(octree_ptr, dest_coord, data_1);
  float value_1 = get_field(data_1);

  Eigen::Vector3f source_point_M = (source_coord.cast<float>() + Eigen::Vector3f::Constant(0.5f));
  Eigen::Vector3f dest_point_M   = (dest_coord.cast<float>() + Eigen::Vector3f::Constant(0.5f));

  return source_point_M + (0.0 - value_0) * (dest_point_M - source_point_M) / (value_1 - value_0);
}

template <typename OctreeT>
inline Eigen::Vector3f interp_vertexes(const std::shared_ptr<OctreeT> octree_ptr,
                                       const unsigned                 x,
                                       const unsigned                 y,
                                       const unsigned                 z,
                                       const int                      edge)
{
  switch(edge){
    case 0:  return compute_intersection(octree_ptr, Eigen::Vector3i(x, y, z),
                                         Eigen::Vector3i(x + 1, y, z));
    case 1:  return compute_intersection(octree_ptr, Eigen::Vector3i(x + 1, y, z),
                                         Eigen::Vector3i(x + 1, y, z + 1));
    case 2:  return compute_intersection(octree_ptr, Eigen::Vector3i(x + 1, y, z + 1),
                                         Eigen::Vector3i(x, y, z + 1));
    case 3:  return compute_intersection(octree_ptr, Eigen::Vector3i(x, y, z),
                                         Eigen::Vector3i(x, y, z + 1));
    case 4:  return compute_intersection(octree_ptr, Eigen::Vector3i(x, y + 1, z),
                                         Eigen::Vector3i(x + 1, y + 1, z));
    case 5:  return compute_intersection(octree_ptr, Eigen::Vector3i(x + 1, y + 1, z),
                                         Eigen::Vector3i(x + 1, y + 1, z + 1));
    case 6:  return compute_intersection(octree_ptr, Eigen::Vector3i(x + 1, y + 1, z + 1),
                                         Eigen::Vector3i(x, y + 1, z + 1));
    case 7:  return compute_intersection(octree_ptr, Eigen::Vector3i(x, y + 1, z),
                                         Eigen::Vector3i(x, y + 1, z + 1));

    case 8:  return compute_intersection(octree_ptr, Eigen::Vector3i(x, y, z),
                                         Eigen::Vector3i(x, y + 1, z));
    case 9:  return compute_intersection(octree_ptr, Eigen::Vector3i(x + 1, y, z),
                                         Eigen::Vector3i(x + 1, y + 1, z));
    case 10: return compute_intersection(octree_ptr, Eigen::Vector3i(x + 1, y, z + 1),
                                         Eigen::Vector3i(x + 1, y + 1, z + 1));
    case 11: return compute_intersection(octree_ptr, Eigen::Vector3i(x, y, z + 1),
                                         Eigen::Vector3i(x, y + 1, z + 1));
  }
  return Eigen::Vector3f::Constant(0);
}

template <typename BlockT>
inline void gather_data(const BlockT*             block_ptr,
                        typename BlockT::DataType data_arr[8],
                        const int                 x,
                        const int                 y,
                        const int                 z)
{
  typename BlockT::DataType data;

  block_ptr->getData(Eigen::Vector3i(x,     y,     z    ), data);
  data_arr[0] = data;
  block_ptr->getData(Eigen::Vector3i(x + 1, y,     z    ), data);
  data_arr[1] = data;
  block_ptr->getData(Eigen::Vector3i(x + 1, y,     z + 1), data);
  data_arr[2] = data;
  block_ptr->getData(Eigen::Vector3i(x,     y,     z + 1), data);
  data_arr[3] = data;
  block_ptr->getData(Eigen::Vector3i(x,     y + 1, z    ), data);
  data_arr[4] = data;
  block_ptr->getData(Eigen::Vector3i(x + 1, y + 1, z    ), data);
  data_arr[5] = data;
  block_ptr->getData(Eigen::Vector3i(x + 1, y + 1, z + 1), data);
  data_arr[6] = data;
  block_ptr->getData(Eigen::Vector3i(x,     y + 1, z + 1), data);
  data_arr[7] = data;
}



template <typename OctreeT>
inline void gather_data(const std::shared_ptr<OctreeT> octree_ptr,
                        typename OctreeT::DataType     data[8],
                        const int                      x,
                        const int                      y,
                        const int                      z)
{
  se::visitor::getData(octree_ptr, Eigen::Vector3i(x,     y,     z    ), data[0]);
  se::visitor::getData(octree_ptr, Eigen::Vector3i(x + 1, y,     z    ), data[1]);
  se::visitor::getData(octree_ptr, Eigen::Vector3i(x + 1, y,     z + 1), data[2]);
  se::visitor::getData(octree_ptr, Eigen::Vector3i(x ,    y,     z + 1), data[3]);
  se::visitor::getData(octree_ptr, Eigen::Vector3i(x,     y + 1, z    ), data[4]);
  se::visitor::getData(octree_ptr, Eigen::Vector3i(x + 1, y + 1, z    ), data[5]);
  se::visitor::getData(octree_ptr, Eigen::Vector3i(x + 1, y + 1, z + 1), data[6]);
  se::visitor::getData(octree_ptr, Eigen::Vector3i(x,     y + 1, z + 1), data[7]);
}



template <typename OctreeT>
uint8_t compute_index(const std::shared_ptr<OctreeT>     octree_ptr,
                      const typename OctreeT::BlockType* block_ptr,
                      const unsigned                     x,
                      const unsigned                     y,
                      const unsigned                     z)
{
  unsigned int block_size =  block_ptr->getSize();
  unsigned int local = ((x % block_size == block_size - 1) << 2) |
                       ((y % block_size == block_size - 1) << 1) |
                       ((z % block_size) == block_size - 1);

  typename OctreeT::DataType data[8];
  if(!local) gather_data(block_ptr, data, x, y, z);
  else gather_data(octree_ptr, data, x, y, z);

  uint8_t index = 0;

  if(is_invalid(data[0])) return 0;
  if(is_invalid(data[1])) return 0;
  if(is_invalid(data[2])) return 0;
  if(is_invalid(data[3])) return 0;
  if(is_invalid(data[4])) return 0;
  if(is_invalid(data[5])) return 0;
  if(is_invalid(data[6])) return 0;
  if(is_invalid(data[7])) return 0;

  if(is_inside(data[0])) index |= 1;
  if(is_inside(data[1])) index |= 2;
  if(is_inside(data[2])) index |= 4;
  if(is_inside(data[3])) index |= 8;
  if(is_inside(data[4])) index |= 16;
  if(is_inside(data[5])) index |= 32;
  if(is_inside(data[6])) index |= 64;
  if(is_inside(data[7])) index |= 128;
  // std::cerr << std::endl << std::endl;

  return index;
}



/// Multi-res marching cube implementation

inline Eigen::Vector3f compute_dual_intersection(const float            value_0,
                                                 const float            value_1,
                                                 const Eigen::Vector3f& dual_corner_coord_0,
                                                 const Eigen::Vector3f& dual_corner_coord_1,
                                                 const int              /* edge_case */) {
  Eigen::Vector3f dual_point_0_M = dual_corner_coord_0;
  Eigen::Vector3f dual_point_1_M = dual_corner_coord_1;
  float iso_value = 0.f;
  return dual_point_0_M + (iso_value - value_0) * (dual_point_1_M - dual_point_0_M) / (value_1 - value_0);
}



template <typename DataT>
inline Eigen::Vector3f interp_dual_vertexes(const int                                   edge,
                                            const DataT                                 data[8],
                                            const std::vector<Eigen::Vector3f,
                                            Eigen::aligned_allocator<Eigen::Vector3f>>& dual_corner_coords_f) {
  switch(edge){
    case 0:  return compute_dual_intersection(get_field(data[0]), get_field(data[1]),
                                              dual_corner_coords_f[0], dual_corner_coords_f[1], 0);
    case 1:  return compute_dual_intersection(get_field(data[1]), get_field(data[2]),
                                              dual_corner_coords_f[1], dual_corner_coords_f[2], 1);
    case 2:  return compute_dual_intersection(get_field(data[2]), get_field(data[3]),
                                              dual_corner_coords_f[2], dual_corner_coords_f[3], 2);
    case 3:  return compute_dual_intersection(get_field(data[0]), get_field(data[3]),
                                              dual_corner_coords_f[0], dual_corner_coords_f[3], 3);
    case 4:  return compute_dual_intersection(get_field(data[4]), get_field(data[5]),
                                              dual_corner_coords_f[4], dual_corner_coords_f[5], 4);
    case 5:  return compute_dual_intersection(get_field(data[5]), get_field(data[6]),
                                              dual_corner_coords_f[5], dual_corner_coords_f[6], 5);
    case 6:  return compute_dual_intersection(get_field(data[6]), get_field(data[7]),
                                              dual_corner_coords_f[6], dual_corner_coords_f[7], 6);
    case 7:  return compute_dual_intersection(get_field(data[4]), get_field(data[7]),
                                              dual_corner_coords_f[4], dual_corner_coords_f[7], 7);
    case 8:  return compute_dual_intersection(get_field(data[0]), get_field(data[4]),
                                              dual_corner_coords_f[0], dual_corner_coords_f[4], 8);
    case 9:  return compute_dual_intersection(get_field(data[1]), get_field(data[5]),
                                              dual_corner_coords_f[1], dual_corner_coords_f[5], 9);
    case 10: return compute_dual_intersection(get_field(data[2]), get_field(data[6]),
                                              dual_corner_coords_f[2], dual_corner_coords_f[6], 10);
    case 11: return compute_dual_intersection(get_field(data[3]), get_field(data[7]),
                                              dual_corner_coords_f[3], dual_corner_coords_f[7], 11);
  }
  return Eigen::Vector3f::Constant(0);
}



template <typename BlockT,
        typename DataT
>
inline void gather_dual_data(const BlockT*                block_ptr,
                             const int                    scale,
                             const Eigen::Vector3f&       primal_corner_coord_f,
                             DataT                        data_arr[8],
                             std::vector<Eigen::Vector3f,
                             Eigen::aligned_allocator<Eigen::Vector3f>>& dual_corner_coords_f)
{
  // In the local case:        actual_dual_offset = actual_dual_scaling * norm_dual_offset_f and
  // dual_corner_coords_f = primal_corner_coord_f + actual_dual_scaling * norm_dual_offset_f
  const float actual_dual_scaling = (float) (1 << scale) / 2;
  for (int corner_idx = 0; corner_idx < 8; corner_idx++) {
    dual_corner_coords_f[corner_idx] = primal_corner_coord_f +
                                       actual_dual_scaling * norm_dual_offset_f[corner_idx];
    typename BlockT::DataType data;
    block_ptr->getData(dual_corner_coords_f[corner_idx].cast<int>(), data); /// <- TODO: Should take data from current scale
    data_arr[corner_idx] = data;
  }
}

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
inline void norm_dual_corner_idxs(const Eigen::Vector3i&         primal_corner_coord_rel,
                                  const int                      block_size,
                                  std::vector<int>&              lower_priority_neighbours,
                                  std::vector<int>&              higher_priority_neighbours,
                                  std::vector<std::vector<int>>& neighbours)
{

  // 26 binary cases (6 faces, 8 corners, 12 edges)
  // 100 000 upper x crossing
  // 010 000 upper y crossing
  // 001 000 upper z crossing

  // 000 100 lower x crossing
  // 000 010 lower y crossing
  // 000 001 lower z crossing

  unsigned int crossmask = ((primal_corner_coord_rel.x() == block_size) << 5) |
                           ((primal_corner_coord_rel.y() == block_size) << 4) |
                           ((primal_corner_coord_rel.z() == block_size) << 3) |
                           ((primal_corner_coord_rel.x() == 0) << 2) |
                           ((primal_corner_coord_rel.y() == 0) << 1) |
                           ( primal_corner_coord_rel.z() == 0);

  switch(crossmask) {
    // 6 Faces
    case 1: /* CASE 1 = crosses lower z */ {
      // Inside
      // (c3;v1)-{-1, -1, +1} and (c7;v3)-{-1, +1, +1} and (c2;v5)-{+1, -1, +1} and (c6;v7)-{+1, +1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1} and (c4;v2)-{-1, +1, -1} and (c1;v4)-{+1, -1, -1} and (c5;v6)-{+1, +1, -1}
      // Higher priority
      // -
      lower_priority_neighbours = {0};
      higher_priority_neighbours = {};
      neighbours = {{2,3,6,7}, {0,1,4,5}};
    }
      break;
    case 2: /* CASE 2 = crosses lower y */ {
      // Inside
      // (c4;v2)-{-1, +1, -1} and (c7;v3)-{-1, +1, +1} and (c5;v6)-{+1, +1, -1} and (c6;v7)-{+1, +1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1} and (c3;v1)-{-1, -1, +1} and (c1;v4)-{+1, -1, -1} and (c2;v5)-{+1, -1, +1}
      // Higher priority
      // -
      lower_priority_neighbours = {0};
      higher_priority_neighbours = {};
      neighbours = {{4,5,6,7}, {0,1,2,3}};
    }
      break;
    case 4: /* CASE 3 = crosses lower x */ {
      // Inside
      // (c1;v4)-{+1, -1, -1} and (c2;v5)-{+1, -1, +1} and (c5;v6)-{+1, +1, -1} and (c6;v7)-{+1, +1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1} and (c3;v1)-{-1, -1, +1} and (c4;v2)-{-1, +1, -1} and (c7;v3)-{-1, +1, +1}
      // Higher priority
      // -
      lower_priority_neighbours = {0};
      higher_priority_neighbours = {};
      neighbours = {{1,2,5,6}, {0,3,4,7}};
    }
      break;
    case 8: /* CASE 4 = crosses upper z */ {
      // Inside
      // (c0;v0)-{-1, -1, -1} and (c4;v2)-{-1, +1, -1} and (c1;v4)-{+1, -1, -1} and (c5;v6)-{+1, +1, -1}
      // Lower priority
      // -
      // Higher priority
      // (c3;v1)-{-1, -1, +1} and (c7;v3)-{-1, +1, +1} and (c2;v5)-{+1, -1, +1} and (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {};
      higher_priority_neighbours = {3};
      neighbours = {{0,1,4,5}, {2,3,6,7}};
    }
      break;
    case 16: /* CASE 5 = crosses upper y */ {
      // Inside
      // (c0;v0)-{-1, -1, -1} and (c3;v1)-{-1, -1, +1} and (c1;v4)-{+1, -1, -1} and (c2;v5)-{+1, -1, +1}
      // Lower priority
      // -
      // Higher priority
      // (c4;v2)-{-1, +1, -1} and (c7;v3)-{-1, +1, +1} and (c5;v6)-{+1, +1, -1} and (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {};
      higher_priority_neighbours = {4};
      neighbours = {{0,1,2,3}, {4,5,6,7}};
    }
      break;
    case 32: /* CASE 6 = crosses upper x */ {
      // Inside
      // (c0;v0)-{-1, -1, -1} and (c3;v1)-{-1, -1, +1} and (c4;v2)-{-1, +1, -1} and (c7;v3)-{-1, +1, +1}
      // Lower priority
      // -
      // Higher priority
      // (c1;v4)-{+1, -1, -1} and (c2;v5)-{+1, -1, +1} and (c5;v6)-{+1, +1, -1} and (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {};
      higher_priority_neighbours = {1};
      neighbours = {{0,3,4,7}, {1,2,5,6}};
    }
      break;


      // 8 Corners
    case 7: /* CASE 7 = crosses lower x(4), y(2), z(1) */ {
      // Inside
      // (c6;v7)-{+1, +1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1}, (c3;v1)-{-1, -1, +1}, (c4;v2)-{-1, +1, -1}, (c7;v3)-{-1, +1, +1}
      // (c1;v4)-{+1, -1, -1}, (c2;v5)-{+1, -1, +1}, (c5;v6)-{+1, +1, -1},
      // Higher priority
      // -
      lower_priority_neighbours = {0, 1, 2, 3, 4, 5, 7};
      higher_priority_neighbours = {};
      neighbours = {{6}, {0}, {1}, {2}, {3}, {4}, {5}, {7}};
    }
      break;
    case 14: /* CASE 8 = crosses lower x(4), y(2) and upper z(8) */ {
      // Inside
      // (c5;v6)-{+1, +1, -1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1}, (c3;v1)-{-1, -1, +1}, (c4;v2)-{-1, +1, -1},
      // (c7;v3)-{-1, +1, +1}, (c1;v4)-{+1, -1, -1}, (c2;v5)-{+1, -1, +1},
      // Higher priority
      // (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {0,1,2,3,4,7};
      higher_priority_neighbours = {6};
      neighbours = {{5}, {0}, {1}, {2}, {3}, {4}, {6}, {7}};
    }
      break;
    case 21: /* CASE 9 = crosses lower x(4), upper y(16) and lower z(1) */ {
      // Inside
      // (c2;v5)-{+1, -1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1}, (c3;v1)-{-1, -1, +1}, (c4;v2)-{-1, +1, -1},
      // (c7;v3)-{-1, +1, +1}, (c1;v4)-{+1, -1, -1},
      // Higher priority
      // (c5;v6)-{+1, +1, -1}, (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {0,1,3,4,7};
      higher_priority_neighbours = {5,6};
      neighbours = {{2}, {0}, {1}, {3}, {4}, {5}, {6}, {7}};
    }
      break;
    case 28: /* CASE 10 = crosses lower x(4) and upper y(16), z(8) */ {
      // Inside
      // (c1;v4)-{+1, -1, -1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1}, (c3;v1)-{-1, -1, +1},
      // (c4;v2)-{-1, +1, -1}, (c7;v3)-{-1, +1, +1}
      // Higher priority
      // (c2;v5)-{+1, -1, +1}, (c5;v6)-{+1, +1, -1}, (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {0,3,4,7};
      higher_priority_neighbours = {2,5,6};
      neighbours = {{1}, {0}, {2}, {3}, {4}, {5}, {6}, {7}};
    }
      break;
    case 35: /* CASE 11 = crosses upper x(32) and lower y(2), z(1) */ {
      // Inside
      // (c7;v3)-{-1, +1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1}, (c3;v1)-{-1, -1, +1}, (c4;v2)-{-1, +1, -1}
      // Higher priority
      // (c1;v4)-{+1, -1, -1}, (c2;v5)-{+1, -1, +1}, (c5;v6)-{+1, +1, -1}, (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {0,3,4};
      higher_priority_neighbours = {1,2,5,6};
      neighbours = {{7}, {0}, {1}, {2}, {3}, {4}, {5}, {6}};
    }
      break;
    case 42: /* CASE 12 = crosses upper x(32), lower y(2) and upper z(8) */ {
      // Inside
      // (c4;v2)-{-1, +1, -1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1}, (c3;v1)-{-1, -1, +1}
      // Higher priority
      // (c7;v3)-{-1, +1, +1}, (c1;v4)-{+1, -1, -1}, (c2;v5)-{+1, -1, +1},
      // (c5;v6)-{+1, +1, -1}, (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {0,3};
      higher_priority_neighbours = {1,2,5,6,7};
      neighbours = {{4}, {0}, {1}, {2}, {3}, {5}, {6}, {7}};
    }
      break;
    case 49: /* CASE 13 = crosses upper x(32), y(16) and lower z(1) */ {
      // Inside
      // (c3;v1)-{-1, -1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1}
      // Higher priority
      // (c4;v2)-{-1, +1, -1}, (c7;v3)-{-1, +1, +1}, (c1;v4)-{+1, -1, -1},
      // (c2;v5)-{+1, -1, +1}, (c5;v6)-{+1, +1, -1}, (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {0};
      higher_priority_neighbours = {1,2,4,5,6,7};
      neighbours = {{3}, {0}, {1}, {2}, {4}, {5}, {6}, {7}};
    }
      break;
    case 56: /* CASE 14 = crosses upper x(32), y(16), z(8) */ {
      // Inside
      // (c0;v0)-{-1, -1, -1}
      // Lower priority
      // -
      // Higher priority
      // (c3;v1)-{-1, -1, +1}, (c4;v2)-{-1, +1, -1}, (c7;v3)-{-1, +1, +1}
      // (c1;v4)-{+1, -1, -1}, (c2;v5)-{+1, -1, +1}, (c5;v6)-{+1, +1, -1}, (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {};
      higher_priority_neighbours = {1, 2, 3, 4, 5, 6, 7};
      neighbours = {{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}};
    }
      break;


      // 12 Edges
    case 3: /* CASE 15 = crosses lower y(2), z(1) */ {
      // Inside
      // (c7;v3)-{-1, +1, +1} and (c6;v7)-{+1, +1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1} and (c1;v4)-{+1, -1, -1}
      // (c3;v1)-{-1, -1, +1} and (c2;v5)-{+1, -1, +1}
      // (c4;v2)-{-1, +1, -1} and (c5;v6)-{+1, +1, -1}
      // Higher priority
      // -
      lower_priority_neighbours = {0,3,4};
      higher_priority_neighbours = {};
      neighbours = {{6,7}, {0,1}, {2,3}, {4,5}};
    }
      break;
    case 5: /* CASE 16 = crosses lower x(4), z(1) */ {
      // Inside
      // (c2;v5)-{+1, -1, +1} and (c6;v7)-{+1, +1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1} and (c4;v2)-{-1, +1, -1}
      // (c3;v1)-{-1, -1, +1} and (c7;v3)-{-1, +1, +1}
      // (c1;v4)-{+1, -1, -1} and (c5;v6)-{+1, +1, -1}
      // Higher priority
      // -
      lower_priority_neighbours = {0,1,3};
      higher_priority_neighbours = {};
      neighbours = {{2,6}, {0,4}, {3,7}, {1,5}};
    }
      break;
    case 6: /* CASE 17 = crosses lower x(4), y(2) */ {
      // Inside
      // (c5;v6)-{+1, +1, -1} and (c6;v7)-{+1, +1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1} and (c3;v1)-{-1, -1, +1}
      // (c4;v2)-{-1, +1, -1} and (c7;v3)-{-1, +1, +1}
      // (c1;v4)-{+1, -1, -1} and (c2;v5)-{+1, -1, +1}
      // Higher priority
      // -
      lower_priority_neighbours = {0,1,4};
      higher_priority_neighbours = {};
      neighbours = {{5,6}, {0,3}, {4,7}, {1,2}};
    }
      break;
    case 10: /* CASE 18 = crosses lower y(2) and upper z(8) */ {
      // Inside
      // (c4;v2)-{-1, +1, -1} and (c5;v6)-{+1, +1, -1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1} and (c1;v4)-{+1, -1, -1}
      // (c3;v1)-{-1, -1, +1} and (c2;v5)-{+1, -1, +1}
      // (c7;v3)-{-1, +1, +1} and (c6;v7)-{+1, +1, +1}
      // Higher priority
      //
      lower_priority_neighbours = {0,3};
      higher_priority_neighbours = {7};
      neighbours = {{4,5}, {0,1}, {2,3}, {6,7}};
    }
      break;
    case 12: /* CASE 19 = crosses lower x(4) and upper z(8) */ {
      // Inside
      // (c1;v4)-{+1, -1, -1} and (c5;v6)-{+1, +1, -1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1} and (c4;v2)-{-1, +1, -1}
      // (c3;v1)-{-1, -1, +1} and (c7;v3)-{-1, +1, +1}
      // Higher priority
      // (c2;v5)-{+1, -1, +1} and (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {0,3};
      higher_priority_neighbours = {2};
      neighbours = {{1,5}, {0,4}, {3,7}, {2,6}};
    }
      break;
    case 17: /* CASE 20 = crosses upper y(16) and lower z(1) */ {
      // Inside
      // (c3;v1)-{-1, -1, +1} and (c2;v5)-{+1, -1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1} and (c1;v4)-{+1, -1, -1}
      // Higher priority
      // (c4;v2)-{-1, +1, -1} and (c5;v6)-{+1, +1, -1}
      // (c7;v3)-{-1, +1, +1} and (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {0};
      higher_priority_neighbours = {4,7};
      neighbours = {{2,3}, {0,1}, {4,5}, {6,7}};
    }
      break;
    case 20: /* CASE 21 = crosses lower x(4) and upper y(16)*/ {
      // Inside
      // (c1;v4)-{+1, -1, -1} and (c2;v5)-{+1, -1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1} and (c3;v1)-{-1, -1, +1}
      // (c4;v2)-{-1, +1, -1} and (c7;v3)-{-1, +1, +1}
      // Higher priority
      // (c5;v6)-{+1, +1, -1} and (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {0,4};
      higher_priority_neighbours = {5};
      neighbours = {{1,2}, {0,3}, {4,7}, {5,6}};
    }
      break;
    case 24: /* CASE 22 = crosses upper y(16), z(8)*/ {
      // Inside
      // (c0;v0)-{-1, -1, -1} and (c1;v4)-{+1, -1, -1}
      // Lower priority
      // -
      // Higher priority
      // (c3;v1)-{-1, -1, +1} and (c2;v5)-{+1, -1, +1}
      // (c4;v2)-{-1, +1, -1} and (c5;v6)-{+1, +1, -1}
      // (c7;v3)-{-1, +1, +1} and (c6;v7)-{+1, +1, +1}
      //
      lower_priority_neighbours = {};
      higher_priority_neighbours = {3,4,7};
      neighbours = {{0,1}, {2,3}, {4,5}, {6,7}};
    }
      break;
    case 33: /* CASE 23 = crosses upper x(32) and lower z(1) */ {
      // Inside
      // (c3;v1)-{-1, -1, +1} and (c7;v3)-{-1, +1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1} and (c4;v2)-{-1, +1, -1}
      // Higher priority
      // (c1;v4)-{+1, -1, -1} and (c5;v6)-{+1, +1, -1}
      // (c2;v5)-{+1, -1, +1} and (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {0};
      higher_priority_neighbours = {1,2};
      neighbours = {{3,7}, {0,4}, {1,5}, {2,6}};
    }
      break;
    case 34: /* CASE 24 = crosses upper x(32) and lower y(2) */ {
      // Inside
      // (c4;v2)-{-1, +1, -1} and (c7;v3)-{-1, +1, +1}
      // Lower priority
      // (c0;v0)-{-1, -1, -1} and (c3;v1)-{-1, -1, +1}
      // Higher priority
      // (c1;v4)-{+1, -1, -1} and (c2;v5)-{+1, -1, +1}
      // (c5;v6)-{+1, +1, -1} and (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {0};
      higher_priority_neighbours = {1,5};
      neighbours = {{4,7}, {0,3}, {1,2}, {5,6}};
    }
      break;
    case 40: /* CASE 25 = crosses upper x(32), z(8) */ {
      // Inside
      // (c0;v0)-{-1, -1, -1} and (c4;v2)-{-1, +1, -1}
      // Lower priority
      // -
      // Higher priority
      // (c3;v1)-{-1, -1, +1} and (c7;v3)-{-1, +1, +1}
      // (c1;v4)-{+1, -1, -1} and (c5;v6)-{+1, +1, -1}
      // (c2;v5)-{+1, -1, +1} and (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {};
      higher_priority_neighbours = {1,2,3};
      neighbours = {{0,4}, {3,7}, {1,5}, {2,6}};
    }
      break;
    case 48: /* CASE 26 = crosses upper x(32), y(16) */ {
      // Inside
      // (c0;v0)-{-1, -1, -1} and (c3;v1)-{-1, -1, +1}
      // Lower priority
      // -
      // Higher priority
      // (c4;v2)-{-1, +1, -1} and (c7;v3)-{-1, +1, +1}
      // (c1;v4)-{+1, -1, -1} and (c2;v5)-{+1, -1, +1}
      // (c5;v6)-{+1, +1, -1} and (c6;v7)-{+1, +1, +1}
      lower_priority_neighbours = {};
      higher_priority_neighbours = {1,4,5};
      neighbours = {{0,3}, {4,7}, {1,2}, {5,6}};
    }
  }
}



template <typename  OctreeT,
        typename DataT
>
inline void gather_dual_data(const std::shared_ptr<OctreeT>     octree_ptr,
                             const typename OctreeT::BlockType* block_ptr,
                             const int                          scale,
                             const Eigen::Vector3i&             primal_corner_coord,
                             DataT                              data_arr[8],
                             std::vector<Eigen::Vector3f,
                             Eigen::aligned_allocator<Eigen::Vector3f>>& dual_corner_coords_f)
{
  const Eigen::Vector3i primal_corner_coord_rel = primal_corner_coord - block_ptr->getCoord();

  std::vector<int> lower_priority_neighbours, higher_priority_neighbours;
  std::vector<std::vector<int>> neighbours;
  norm_dual_corner_idxs(primal_corner_coord_rel, block_ptr->getSize(),
                        lower_priority_neighbours, higher_priority_neighbours, neighbours);

  for(const auto& offset_idx: lower_priority_neighbours) {
    Eigen::Vector3i logical_dual_corner_coord = primal_corner_coord + logical_dual_offset[offset_idx];
    if (!octree_ptr->contains(logical_dual_corner_coord)) {
      set_invalid(data_arr[0]);
      return;
    }
    typename OctreeT::BlockType* block_neighbour_ptr = static_cast<typename OctreeT::BlockType*>(se::fetcher::block(logical_dual_corner_coord, octree_ptr, octree_ptr->getRoot()));
    if (block_neighbour_ptr == nullptr || block_ptr->getCurrentScale() <= scale) {
      set_invalid(data_arr[0]);
      return;
    }
  }
  for(const auto& offset_idx: higher_priority_neighbours) {
    Eigen::Vector3i logical_dual_corner_coord = primal_corner_coord + logical_dual_offset[offset_idx];
    if (!octree_ptr->contains(logical_dual_corner_coord)) {
      set_invalid(data_arr[0]);
      return;
    }
    typename OctreeT::BlockType* block_neighbour_ptr = static_cast<typename OctreeT::BlockType*>(se::fetcher::block(logical_dual_corner_coord, octree_ptr, octree_ptr->getRoot()));
    if (block_neighbour_ptr == nullptr || block_ptr->getCurrentScale() < scale) {
      set_invalid(data_arr[0]);
      return;
    }
  }

  int stride = 1 << block_ptr->getCurrentScale();
  for(const auto& offset_idx: neighbours[0]) {
    Eigen::Vector3i logical_dual_corner_coord = primal_corner_coord + logical_dual_offset[offset_idx];
    dual_corner_coords_f[offset_idx] = ((logical_dual_corner_coord / stride) * stride).cast<float>() + stride * Eigen::Vector3f::Constant(0.5f); // TODO:  OctreeT<FieldType>::sample_offset_frac_
    typename OctreeT::DataType data;
    block_ptr->getData(dual_corner_coords_f[offset_idx].cast<int>(), data); /// <- TODO: Should take data from current scale
    data_arr[offset_idx] = data;
  }
  for (size_t neighbour_idx = 1; neighbour_idx < neighbours.size(); ++neighbour_idx) {
    Eigen::Vector3i logical_dual_corner_coord = primal_corner_coord + logical_dual_offset[neighbours[neighbour_idx][0]];
    typename OctreeT::BlockType* block_neighbour_ptr = static_cast<typename OctreeT::BlockType*>(se::fetcher::block(logical_dual_corner_coord, octree_ptr, octree_ptr->getRoot()));
    int stride = 1 << block_neighbour_ptr->getCurrentScale();
    for(const auto& offset_idx: neighbours[neighbour_idx]) {
      logical_dual_corner_coord = primal_corner_coord + logical_dual_offset[offset_idx];
      dual_corner_coords_f[offset_idx] = ((logical_dual_corner_coord / stride) * stride).cast<float>() + stride * Eigen::Vector3f::Constant(0.5f); // TODO: OctreeT<FieldType>::sample_offset_frac_
      typename OctreeT::DataType data;
      block_neighbour_ptr->getData(dual_corner_coords_f[offset_idx].cast<int>(), data); /// <- TODO: Should take data from current scale
      data_arr[offset_idx] = data;
    }
  }
}



template <typename OctreeT,
          typename DataT
>
void compute_dual_index(const std::shared_ptr<OctreeT>     octree_ptr,
                        const typename OctreeT::BlockType* block_ptr,
                        const int                          scale,
                        const Eigen::Vector3i&             primal_corner_coord,
                        uint8_t&                           edge_pattern_idx,
                        DataT                              data[8],
                        std::vector<Eigen::Vector3f,
                        Eigen::aligned_allocator<Eigen::Vector3f>>& dual_corner_coords_f)
{
  const unsigned int block_size =  block_ptr->getSize();
  // The local case is independent of the scale.
  // lower or upper x boundary (block_coord.x() +0 or +block size) -> (binary) 100 -> local += 4
  // lower or upper y boundary (block_coord.y() +0 or +block size) -> (binary) 010 -> local += 2
  // lower or upper z boundary (block_coord.z() +0 or +block size) -> (binary) 001 -> local += 1
  // local = 0 -> local     - dual contains only local primal neightbours
  // local = 1 -> not local - dual crosses z         boundary
  // local = 2 -> not local - dual crosses y         boundary
  // local = 3 -> not local - dual crosses y + z     boundary
  // local = 4 -> not local - dual crosses x         boundary
  // local = 5 -> not local - dual crosses x + z     boundary
  // local = 6 -> not local - dual crosses x + y     boundary
  // local = 7 -> not local - dual crosses x + y + z boundary
  const unsigned int local = ((primal_corner_coord.x() % block_size == 0) << 2) |
                             ((primal_corner_coord.y() % block_size == 0) << 1) |
                             ( primal_corner_coord.z() % block_size == 0);

  edge_pattern_idx = 0;
  if(!local) gather_dual_data(block_ptr, scale, primal_corner_coord.cast<float>(), data, dual_corner_coords_f);
  else gather_dual_data(octree_ptr, block_ptr, scale, primal_corner_coord, data, dual_corner_coords_f);

  // Only compute dual index if all data is valid/observed
  for (int corner_idx = 0; corner_idx < 8; corner_idx++) {
    if(!is_valid(data[corner_idx])) return;
  }

  // if(inside(data[0])) edge_pattern_idx |= 1;
  // if(inside(data[1])) edge_pattern_idx |= 2;
  // if(inside(data[2])) edge_pattern_idx |= 4;
  // if(inside(data[3])) edge_pattern_idx |= 8;
  // if(inside(data[4])) edge_pattern_idx |= 16;
  // if(inside(data[5])) edge_pattern_idx |= 32;
  // if(inside(data[6])) edge_pattern_idx |= 64;
  // if(inside(data[7])) edge_pattern_idx |= 128;
  for (int corner_idx = 0; corner_idx < 8; corner_idx++) {
    if(is_inside(data[corner_idx])) edge_pattern_idx |= (1 << corner_idx);
  }
  // std::cerr << std::endl << std::endl;

}



inline bool checkVertex(const Eigen::Vector3f& vertex_M, const float dim)
{
  return (vertex_M.x() <= 0 || vertex_M.y() <=0 || vertex_M.z() <= 0 ||
          vertex_M.x() > dim || vertex_M.y() > dim || vertex_M.z() > dim);
}



} // namespace meshing



namespace algorithms {

template <typename OctreeT,
          typename TriangleType>
void marching_cube(std::shared_ptr<OctreeT>   octree_ptr,
                   std::vector<TriangleType>& triangles) {

  using namespace meshing;
  typedef typename OctreeT::BlockType BlockType;

  const int block_size   = OctreeT::BlockType::getSize();
  const int octree_size  = octree_ptr->getSize();

//#pragma omp parallel for
  for (auto block_ptr_itr = se::BlocksIterator<OctreeT>(octree_ptr.get()); block_ptr_itr != se::BlocksIterator<OctreeT>(); ++block_ptr_itr)
  {
    BlockType* block_ptr   = static_cast<BlockType*>(*block_ptr_itr);

    const Eigen::Vector3i& start_coord = block_ptr->getCoord();
    const Eigen::Vector3i last_coord =
            (start_coord + Eigen::Vector3i::Constant(block_size)).cwiseMin(
                    Eigen::Vector3i::Constant(octree_size - 1));
    for (int x = start_coord.x(); x < last_coord.x(); x++)
    {
      for (int y = start_coord.y(); y < last_coord.y(); y++)
      {
        for (int z = start_coord.z(); z < last_coord.z(); z++)
        {

          const uint8_t edge_pattern_idx = meshing::compute_index(octree_ptr, block_ptr, x, y, z);
          const int* edges = triTable[edge_pattern_idx];
          for (unsigned int e = 0; edges[e] != -1 && e < 16; e += 3)
          {
            Eigen::Vector3f vertex_0 = interp_vertexes(octree_ptr, x, y, z, edges[e]);
            Eigen::Vector3f vertex_1 = interp_vertexes(octree_ptr, x, y, z, edges[e + 1]);
            Eigen::Vector3f vertex_2 = interp_vertexes(octree_ptr, x, y, z, edges[e + 2]);

            if (checkVertex(vertex_0, octree_size) || checkVertex(vertex_1, octree_size) || checkVertex(vertex_2, octree_size))
            {
              continue;
            }

            Triangle temp = Triangle();
            temp.vertexes[0] = vertex_0;
            temp.vertexes[1] = vertex_1;
            temp.vertexes[2] = vertex_2;
            triangles.push_back(temp);
          } // edges

        } // z
      } // y
    } // x

  } // block_ptr_itr
}

template <typename OctreeT,
          typename TriangleT
>
void dual_marching_cube(std::shared_ptr<OctreeT> octree_ptr,
                        std::vector<TriangleT>&   triangles)
{
  using namespace meshing;
  typedef typename OctreeT::BlockType BlockType;

  const int block_size   = OctreeT::BlockType::getSize();
  const int octree_size  = octree_ptr->getSize();

  for (auto block_ptr_itr = se::BlocksIterator<OctreeT>(octree_ptr.get()); block_ptr_itr != se::BlocksIterator<OctreeT>(); ++block_ptr_itr)
  {
    BlockType* block_ptr   = static_cast<BlockType*>(*block_ptr_itr);
    const int voxel_scale  = block_ptr->getCurrentScale();
    const int voxel_stride = 1 << voxel_scale;
    const Eigen::Vector3i& start_coord = block_ptr->getCoord();
    const Eigen::Vector3i last_coord =
            (start_coord + Eigen::Vector3i::Constant(block_size)).cwiseMin(
                    Eigen::Vector3i::Constant(octree_size - 1));
    for (int x = start_coord.x() + 1; x <= last_coord.x(); x += voxel_stride) {
      for (int y = start_coord.y() + 1; y <= last_coord.y(); y += voxel_stride) {
        for (int z = start_coord.z() + 1; z <= last_coord.z(); z += voxel_stride) {
          const Eigen::Vector3i primal_corner_coord = Eigen::Vector3i(x, y, z);

          if (x == last_coord.x() || y == last_coord.y() || z == last_coord.z())
          {
            if(!se::fetcher::block(primal_corner_coord, octree_ptr, octree_ptr->getRoot()))
            {
              continue;
            }
          }

          uint8_t edge_pattern_idx;
          typename OctreeT::DataType data[8];
          std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>> dual_corner_coords_f(8, Eigen::Vector3f::Constant(0));
          meshing::compute_dual_index(octree_ptr, block_ptr, voxel_scale, primal_corner_coord, edge_pattern_idx, data, dual_corner_coords_f);
          const int* edges = triTable[edge_pattern_idx];
          for (unsigned int e = 0; edges[e] != -1 && e < 16; e += 3)
          {
            Eigen::Vector3f vertex_0 = interp_dual_vertexes(edges[e], data, dual_corner_coords_f);
            Eigen::Vector3f vertex_1 = interp_dual_vertexes(edges[e + 1], data, dual_corner_coords_f);
            Eigen::Vector3f vertex_2 = interp_dual_vertexes(edges[e + 2], data, dual_corner_coords_f);

            if (checkVertex(vertex_0, octree_size) || checkVertex(vertex_1, octree_size) || checkVertex(vertex_2, octree_size))
            {
              continue;
            }
            Triangle temp = Triangle();
            temp.vertexes[0] = vertex_0;
            temp.vertexes[1] = vertex_1;
            temp.vertexes[2] = vertex_2;
            triangles.push_back(temp);
          } // edges

        } // z
      } // y
    } // x

  } // block_ptr_itr
}

} // namespace algorithms
} // namespace se

#endif //SE_MARCHING_CUBE_IMPL_HPP
