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

#ifndef PONOS_BLAS_FDM_MATRIX_H
#define PONOS_BLAS_FDM_MATRIX_H

#include <ponos/blas/fdm_vector.h>

namespace ponos {

template <typename T> struct FDMMatrixRow2D {
  T center;
  T right;
  T up;
};

template <typename T> struct FDMMatrixRow3D {
  T center;
  T right;
  T up;
  T front;
};

typedef FDMVector2D<FDMMatrixRow2D<float>> FDMMatrix2Df;
typedef FDMVector3D<FDMMatrixRow3D<float>> FDMMatrix3Df;

} // ponos namespace

#endif // PONOS_BLAS_FDM_MATRIX_H
