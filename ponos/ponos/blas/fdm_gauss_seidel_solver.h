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

#ifndef PONOS_BLAS_GAUSS_SEIDEL_SOLVER_H
#define PONOS_BLAS_GAUSS_SEIDEL_SOLVER_H

#include <ponos/blas/blas.h>
#include <ponos/blas/linear_system.h>
#include <ponos/blas/solver_interface.h>

namespace ponos {

class FDMGaussSeidelSolver2f : public LinearSystemSolver<FDMLinearSystem2f> {
public:
  bool solve(FDMLinearSystem2f *s) override {
    residual.resize(s->x.width, s->x.height);
    for (uint it = 0; it < maxNumberOfIterations; it++) {
      relax(s);
      if (it != 0 && it % residualCheckInterval == 0) {
        FDMBlas2f::residual(s->A, s->x, s->b, &residual);
        if (FDMBlas2f::l2Norm(residual) < tolerance) {
          break;
        }
      }
    }
    FDMBlas2f::residual(s->A, s->x, s->b, &residual);
    lastResidual = FDMBlas2f::l2Norm(residual);
    return lastResidual < tolerance;
  }

private:
  uint maxNumberOfIterations;
  uint lastNumberOfIterations;
  uint residualCheckInterval;
  double tolerance;
  double lastResidual;

  FDMVector2Df residual;

  void relax(FDMLinearSystem2f *s) {
    FDMMatrix2Df &A = s->A;
    FDMVector2Df &x = s->x;
    FDMVector2Df &b = s->b;
    size_t w = A.width, h = A.height;
    for (size_t i = 0; i < w; i++)
      for (size_t j = 0; j < h; j++) {
        double r = ((i > 0) ? A(i - 1, j).right * x(i - 1, j) : 0.0) +
                   ((i + 1 < w) ? A(i, j).right * x(i + 1, j) : 0.0) +
                   ((j > 0) ? A(i, j - 1).up * x(i, j - 1) : 0.0) +
                   ((j + 1 < h) ? A(i, j).up * x(i, j + 1) : 0.0);
        x(i, j) = (b(i, j) - r) / A(i, j).center;
      }
  }
};

} // ponos namespace

#endif // PONOS_BLAS_GAUSS_SEIDEL_SOLVER_H
