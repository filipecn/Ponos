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

#ifndef PONOS_BLAS_CG_SOLVER_H
#define PONOS_BLAS_CG_SOLVER_H

#include <ponos/blas/blas.h>
#include <ponos/blas/linear_system.h>
#include <ponos/blas/pcg.h>
#include <ponos/blas/solver_interface.h>

namespace ponos {

template <typename Preconditioner =
              IncompleteCholeskyCGPreconditioner<FDMBlas2f>>
class FDMCGSolver2f : public LinearSystemSolver<FDMLinearSystem2f> {
public:
  FDMCGSolver2f(uint _maxNumberOfIterations, double _tolerance)
      : maxNumberOfIterations(_maxNumberOfIterations), tolerance(_tolerance) {}
  bool solve(FDMLinearSystem2f *system) override {
    FDMMatrix2Df &matrix = system->A;
    FDMVector2Df &solution = system->x;
    FDMVector2Df &rhs = system->b;

    r.resize(matrix.width, matrix.height);
    d.resize(matrix.width, matrix.height);
    q.resize(matrix.width, matrix.height);
    s.resize(matrix.width, matrix.height);

    system->x.set(0.0);
    r.set(0.0);
    d.set(0.0);
    q.set(0.0);
    s.set(0.0);

    precond.build(matrix);

    pcg<FDMBlas2f, Preconditioner>(matrix, rhs, maxNumberOfIterations,
                                   tolerance, &precond, &solution, &r, &d, &q,
                                   &s, &lastNumberOfIterations, &lastResidual);

    return lastResidual <= tolerance ||
           lastNumberOfIterations < maxNumberOfIterations;
  }

  uint lastNumberOfIterations;
  double lastResidual;

private:
  uint maxNumberOfIterations;
  double tolerance;

  FDMVector2Df r, d, q, s;
  Preconditioner precond;
};

} // ponos namespace

#endif // PONOS_BLAS_CG_SOLVER_H
