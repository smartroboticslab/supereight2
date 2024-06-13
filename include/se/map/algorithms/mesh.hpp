/*
 * SPDX-FileCopyrightText: 2016-2019 Emanuele Vespa
 * SPDX-FileCopyrightText: 2019-2021 Smart Robotics Lab, Imperial College London, Technical University of Munich
 * SPDX-FileCopyrightText: 2021 Nils Funk
 * SPDX-FileCopyrightText: 2021 Sotiris Papatheodorou
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SE_MESH_FACE_HPP
#define SE_MESH_FACE_HPP

#include <Eigen/Core>
#include <Eigen/StdVector>
#include <array>
#include <cstdint>
#include <optional>
#include <se/common/rgb.hpp>
#include <vector>

namespace se {

template<size_t NumVertexes>
struct MeshFace {
    std::array<Eigen::Vector3f, NumVertexes> vertexes;
    std::int8_t scale = 0;

    static constexpr size_t num_vertexes = NumVertexes;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

/** \brief Meshes are represented as lists of faces.
 *
 * \bug This representation has the inherent problem that there is vertex duplication. A more
 * advanced representation would be needed to alleviate this, e.g. a list of vertices and a list of
 * faces with indices to the list of faces.
 */
template<typename FaceT>
using Mesh = std::vector<FaceT>;

typedef MeshFace<3> Triangle;
typedef Mesh<Triangle> TriangleMesh;

typedef MeshFace<4> Quad;
typedef Mesh<Quad> QuadMesh;



/** Return a triangle mesh containig two triangles for each face of \p quad_mesh. */
static inline TriangleMesh quad_to_triangle_mesh(const QuadMesh& quad_mesh);

namespace meshing {

struct Vertex {
    Vertex(const Eigen::Vector3f& position) : position(position)
    {
    }

    Eigen::Vector3f position;
    std::optional<Eigen::Vector3f> normal;
    std::optional<RGB> color;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template<size_t NumFaceVertices = 3>
class VertexIndexMesh {
    public:
    std::vector<Vertex, Eigen::aligned_allocator<Vertex>> vertices;
    std::vector<size_t> indices; // faces

    static constexpr size_t num_face_vertices = NumFaceVertices;

    void merge(const VertexIndexMesh& other);

    void compute_normals();

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

} // namespace meshing
} // namespace se

#include "impl/mesh_impl.hpp"

#endif // SE_MESH_FACE_HPP
