/*
 * Copyright (c) 2017 FilipeCN
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

#ifndef PONOS_BLAS_C_REGULAR_GRID_H
#define PONOS_BLAS_C_REGULAR_GRID_H

#include <ponos/blas/field.h>
#include <ponos/numeric/interpolation.h>
#include <ponos/log/debug.h>
#include <ponos/numeric/grid_interface.h>

#include <algorithm>
#include <memory>
#include <vector>

namespace ponos {

/** Simple matrix structure.
 */
template <typename T = float> class CRegularGrid : public CGridInterface<T> {
public:
  CRegularGrid() {}
  /* Constructor
   * \param d **[in]** dimensions
   * \param b **[in]** background (default value)
   * \param cellSize **[in | optional]** grid spacing
   * \param offset **[in | optional]** grid origin
   */
  CRegularGrid(const size3 &d, const T &b, const vec3 cellSize = vec3(1.f),
               const vec3 &offset = vec3());
  CRegularGrid(const size3 &d, const T &b, const bbox3 &bb);

  ~CRegularGrid();
  /* @inherit */
  void set(const size3 &i, const T &v) override;
  void setAll(T v);
  /* @inherit */
  T operator()(const index3 &i) const override {
    Check::is_between(i[0], 0, static_cast<i32>(this->dimensions[0]));
    Check::is_between(i[1], 0, static_cast<i32>(this->dimensions[1]));
    Check::is_between(i[2], 0, static_cast<i32>(this->dimensions[2]));
    return data[i[0]][i[1]][i[2]];
  }
  /* @inherit */
  T &operator()(const index3 &i) override {
    Check::is_between(i[0], 0, static_cast<i32>(this->dimensions[0]));
    Check::is_between(i[1], 0, static_cast<i32>(this->dimensions[1]));
    Check::is_between(i[2], 0, static_cast<i32>(this->dimensions[2]));
    return data[i[0]][i[1]][i[2]];
  }
  /* @inherit */
  T operator()(const i32 &i, const i32 &j, const i32 &k) const override {
    Check::is_between(i, 0, static_cast<i32>(this->dimensions[0]));
    Check::is_between(j, 0, static_cast<i32>(this->dimensions[1]));
    Check::is_between(k, 0, static_cast<i32>(this->dimensions[2]));
    return data[i][j][k];
  }
  /* @inherit */
  T &operator()(const i32 &i, const i32 &j, const i32 &k) override {
    Check::is_between(i, 0, static_cast<i32>(this->dimensions[0]));
    Check::is_between(j, 0, static_cast<i32>(this->dimensions[1]));
    Check::is_between(k, 0, static_cast<i32>(this->dimensions[2]));
    return data[i][j][k];
  }
  T safeData(int i, int j, int k) const;
  T operator()(const float &x, const float &y, const float &z) const override {
    point3 gp = this->toGrid(ponos::point3(x, y, z));
    float p[3] = {gp.x, gp.y, gp.z};
    return trilinearInterpolate<T>(p, data, this->background, this->dimensions);
  }
  T operator()(const point3 &i) const override {
    return (*this)(i[0], i[1], i[2]);
  }
  void normalize() override;
  void normalizeElements() override;

private:
  T ***data = nullptr;
};
/**
 */
template <typename T = float>
class CRegularGrid2D : public Grid2DInterface<T>,
                       virtual public FieldInterface2D<T> {
public:
  CRegularGrid2D();
  CRegularGrid2D(uint32_t w, uint32_t h);
  virtual ~CRegularGrid2D() {}

  T getData(int i, int j) const override { return data[i][j]; }
  T &getData(int i, int j) override { return data[i][j]; }
  void updateDataStructure() override;
  T sample(float x, float y) const override;

  T infNorm() const {
    T r = std::fabs((*this)(0, 0));
    for (uint32_t i = 0; i < data.size(); i++)
      for (uint32_t j = 0; j < data[i].size(); ++j)
        r = std::max(r, std::fabs(data[i][j]));
    return r;
  }
  void copyData(const CRegularGrid2D<T> *g) {
    for (size_t i = 0; i < data.size(); i++)
      std::copy(g->data[i].begin(), g->data[i].end(), data[i].begin());
  }

protected:
  std::vector<std::vector<T>> data;
};

#include "c_regular_grid.inl"

} // namespace ponos

#endif // PONOS_BLAS_C_REGULAR_GRID_H
