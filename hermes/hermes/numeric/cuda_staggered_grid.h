/*
 * Copyright (c) 2019 FilipeCN
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * iM the Software without restriction, including without limitation the rights
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

#ifndef HERMES_NUMERIC_CUDA_STAGGERED_GRID_H
#define HERMES_NUMERIC_CUDA_STAGGERED_GRID_H

#include <hermes/numeric/cuda_vector_field.h>

namespace hermes {

namespace cuda {

/// Represents a staggered grid with texture grid
/// The origin of each staggered grid follows the scheme:
///    ----------
///   |         |
///   u    o    |
///   |         |
///   -----v----
class StaggeredGridTexture2 : public VectorGridTexture2 {
public:
  StaggeredGridTexture2() {
    uGrid.setOrigin(point2f(-0.5f, 0.0f));
    vGrid.setOrigin(point2f(0.0f, -0.5f));
  }
  /// \param resolution in number of cells
  /// \param origin (0,0) corner position
  /// \param dx cell size
  StaggeredGridTexture2(vec2u resolution, point2f origin, float dx) {
    uGrid.resize(resolution + vec2u(1, 0));
    uGrid.setOrigin(origin + vec2f(-0.5f, 0.f));
    uGrid.setDx(dx);
    vGrid.resize(resolution + vec2u(0, 1));
    vGrid.setOrigin(origin + vec2f(0.f, -0.5f));
    vGrid.setDx(dx);
  }
  /// Changes grid resolution
  /// \param res new resolution (in number of cells)
  void resize(vec2u res) override {
    uGrid.resize(res + vec2u(1, 0));
    vGrid.resize(res + vec2u(0, 1));
  }
  /// Changes grid origin position
  /// \param o in world space
  void setOrigin(const point2f &o) override {
    uGrid.setOrigin(o + vec2f(-0.5f, 0.f));
    vGrid.setOrigin(o + vec2f(0.f, -0.5f));
  }
};

} // namespace cuda

} // namespace hermes

#endif // HERMES_STRUCTURES_CUDA_STAGGERED_GRID_H