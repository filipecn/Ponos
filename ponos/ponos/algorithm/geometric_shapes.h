// Created by filipecn on 3/14/18.
/*
 * Copyright (c) 2018 FilipeCN
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef PONOS_GEOMETRIC_SHAPES_H
#define PONOS_GEOMETRIC_SHAPES_H

#include <ponos/structures/raw_mesh.h>
#include <ponos/geometry/plane.h>

namespace ponos {

/// \param center
/// \param radius
/// \param divisions
/// \param generateNormals
/// \param generateUVs
/// \return raw mesh pointer
RawMesh *create_icosphere_mesh(const point3 &center, real_t radius,
                               size_t divisions, bool generateNormals,
                               bool generateUVs);
/// \param p1
/// \param p2
/// \param p3
/// \param p4
/// \param generateNormals
/// \param generateUVs
/// \return
RawMesh *create_quad_mesh(const point3 &p1 = std::move(point3(0, 0, 0)),
                          const point3 &p2 = std::move(point3(1, 0, 0)),
                          const point3 &p3 = std::move(point3(1, 1, 0)),
                          const point3 &p4 = std::move(point3(0, 1, 0)),
                          bool generateNormals = true, bool generateUVs = true);
/// \param p1
/// \param p2
/// \param p3
/// \param p4
/// \param triangleFaces
/// \return
RawMesh *create_quad_wireframe_mesh(const point3 &p1, const point3 &p2,
                                    const point3 &p3, const point3 &p4,
                                    bool triangleFaces = false);

class RawMeshes {
public:
  static RawMeshSPtr plane(const Plane &plane,
                           const point3& center,
                           const vec3& extension,
                           u32 divisions = 1,
                           bool generate_normals = true,
                           bool generate_uv = true);
  ///
  /// \param radius
  /// \param center
  /// \param divisions
  /// \return
  static RawMeshSPtr circle(real_t radius = 1, const point2 &center = point2(), u32 divisions = 5);
  ///
  /// \param radius
  /// \param center
  /// \param divisions
  /// \return
  static RawMeshSPtr circuference(real_t radius = 1, const point2 &center = point2(), u32 divisions = 10);
  ///
  /// \param a
  /// \param b
  /// \return
  static RawMeshSPtr segment(const point2 &a, const point2 &b = point2());
  ///
  /// \param a
  /// \param b
  /// \return
  static RawMeshSPtr segment(const point3 &a, const point3 &b = point3());
  static RawMeshSPtr icosphere(const point2 &center,
                               real_t radius, size_t divisions,
                               bool generateNormals = false);

  static RawMeshSPtr icosphere(const point3 &center, real_t radius,
                               size_t divisions, bool generateNormals,
                               bool generateUVs);

  ///
  /// \param transform
  /// \param generateNormals
  /// \param generateUVs
  /// \return
  static RawMesh *cube(const ponos::Transform &transform = ponos::Transform(),
                       bool generateNormals = false, bool generateUVs = false);
  ///
  /// \param transform
  /// \param generate_uvs
  /// \return
  static RawMesh *quad(const ponos::Transform &transform = ponos::Transform(),
                       bool generate_uvs = false);

  /// \param transform
  /// \param triangleFaces
  /// \return
  static RawMeshSPtr
  cubeWireframe(const ponos::Transform &transform = ponos::Transform(),
                bool triangleFaces = false);
};

} // namespace ponos

#endif // PONOS_GEOMETRIC_SHAPES_H
