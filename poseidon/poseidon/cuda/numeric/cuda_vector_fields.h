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

#ifndef POSEIDON_SIMULATION_CUDA_VECTOR_FIELDS_H
#define POSEIDON_SIMULATION_CUDA_VECTOR_FIELDS_H

#include <hermes/hermes.h>

namespace poseidon {

namespace cuda {

__host__ __device__ hermes::cuda::vec3f
enrightDeformationField(hermes::cuda::point3f p);

__host__ __device__ hermes::cuda::vec2f
zalesakDeformationField(hermes::cuda::point2f p);

void applyEnrightDeformationField(hermes::cuda::StaggeredGrid3D &grid);

void applyZalesakDeformationField(hermes::cuda::StaggeredGrid2D &grid);

} // namespace cuda

} // namespace poseidon

#endif
