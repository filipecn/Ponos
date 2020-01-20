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

#ifndef HERMES_PARALLEL_CUDA_REDUCE_H
#define HERMES_PARALLEL_CUDA_REDUCE_H

#include <hermes/common/cuda.h>
#include <hermes/common/defs.h>
#include <hermes/storage/cuda_storage_utils.h>

namespace hermes {

namespace cuda {

template <unsigned int blockSize, typename T>
__global__ void __reduceAdd(const T *data, T *rdata, unsigned int n) {
  extern __shared__ T sdata[];
  unsigned int threadId = threadIdx.x;
  unsigned int i = blockIdx.x * (blockSize * 2) + threadId;
  unsigned int gridSize = blockSize * 2 * gridDim.x;
  sdata[threadId] = 0;
  while (i < n) {
    sdata[threadId] += data[i] + data[i + blockSize];
    i += gridSize;
  }
  __syncthreads();

  if (blockSize >= 512) {
    if (threadId < 256)
      sdata[threadId] += sdata[threadId + 256];
    __syncthreads();
  }
  if (blockSize >= 256) {
    if (threadId < 128)
      sdata[threadId] += sdata[threadId + 128];
    __syncthreads();
  }
  if (blockSize >= 128) {
    if (threadId < 64)
      sdata[threadId] += sdata[threadId + 64];
    __syncthreads();
  }
  if (threadId < 32) {
    if (blockSize >= 64)
      sdata[threadId] += sdata[threadId + 32];
    if (blockSize >= 32)
      sdata[threadId] += sdata[threadId + 16];
    if (blockSize >= 16)
      sdata[threadId] += sdata[threadId + 8];
    if (blockSize >= 8)
      sdata[threadId] += sdata[threadId + 4];
    if (blockSize >= 4)
      sdata[threadId] += sdata[threadId + 2];
    if (blockSize >= 2)
      sdata[threadId] += sdata[threadId + 1];
  }
  if (threadId == 0)
    rdata[blockIdx.x] = sdata[0];
}

template <unsigned int blockSize, typename T>
__global__ void __reduceMin(const T *data, T *rdata, unsigned int n) {
  extern __shared__ T sdata[];
  unsigned int threadId = threadIdx.x;
  unsigned int i = blockIdx.x * (blockSize * 2) + threadId;
  unsigned int gridSize = blockSize * 2 * gridDim.x;
  sdata[threadId] = Constants::lowest<T>();
  while (i < n) {
    sdata[threadId] = fminf(data[i], data[i + blockSize]);
    i += gridSize;
  }
  __syncthreads();

  if (blockSize >= 512) {
    if (threadId < 256)
      sdata[threadId] = fminf(sdata[threadId], sdata[threadId + 256]);
    __syncthreads();
  }
  if (blockSize >= 256) {
    if (threadId < 128)
      sdata[threadId] = fminf(sdata[threadId], sdata[threadId + 128]);
    __syncthreads();
  }
  if (blockSize >= 128) {
    if (threadId < 64)
      sdata[threadId] = fminf(sdata[threadId], sdata[threadId + 64]);
    __syncthreads();
  }
  if (threadId < 32) {
    if (blockSize >= 64)
      sdata[threadId] = fminf(sdata[threadId], sdata[threadId + 32]);
    if (blockSize >= 32)
      sdata[threadId] = fminf(sdata[threadId], sdata[threadId + 16]);
    if (blockSize >= 16)
      sdata[threadId] = fminf(sdata[threadId], sdata[threadId + 8]);
    if (blockSize >= 8)
      sdata[threadId] = fminf(sdata[threadId], sdata[threadId + 4]);
    if (blockSize >= 4)
      sdata[threadId] = fminf(sdata[threadId], sdata[threadId + 2]);
    if (blockSize >= 2)
      sdata[threadId] = fminf(sdata[threadId], sdata[threadId + 1]);
  }
  if (threadId == 0)
    rdata[blockIdx.x] = sdata[0];
}

template <typename T> T reduceAdd(const T *data, unsigned int n) {
  unsigned int blockSize = 128;
  unsigned int gridSize = 2;
  T h_r = 0;
  // __reduceAdd<blockSize, T><<<gridSize, blockSize>>>(data, r, n);
  return h_r;
}

template <typename T>
__global__ void k__min(MemoryBlock2Accessor<T> data, T *c) {
  __shared__ float cache[256];

  int tid = blockDim.x * blockIdx.x + threadIdx.x;
  int cacheindex = threadIdx.x;
  int n = data.size().x * data.size().y;

  T temp = 1 << 20;
  while (tid < n) {
    int i = tid / data.size().x;
    int j = tid % data.size().x;
    temp = fminf(temp, data(i, j));
    tid += blockDim.x * gridDim.x;
  }
  cache[cacheindex] = temp;
  __syncthreads();

  int i = blockDim.x / 2;
  while (i != 0) {
    if (cacheindex < i)
      cache[cacheindex] = fminf(cache[cacheindex], cache[cacheindex + i]);
    __syncthreads();
    i /= 2;
  }
  if (cacheindex == 0)
    c[blockIdx.x] = cache[0];
}

template <typename T> T minValue(MemoryBlock2<MemoryLocation::DEVICE, T> &data) {
  size_t blockSize = (data.size().x * data.size().y + 256 - 1) / 256;
  if (blockSize > 32)
    blockSize = 32;
  T *c = new T[blockSize];
  T *d_c;
  cudaMalloc((void **)&d_c, blockSize * sizeof(T));
  k__min<<<blockSize, 256>>>(data.accessor(), d_c);
  cudaMemcpy(c, d_c, blockSize * sizeof(T), cudaMemcpyDeviceToHost);
  T norm = 0;
  for (int i = 0; i < blockSize; i++)
    norm = fmin(norm, c[i]);
  cudaFree(d_c);
  delete[] c;
  return norm;
}

template <typename T>
__global__ void k__max(MemoryBlock2Accessor<T> data, T *c) {
  __shared__ float cache[256];

  int tid = blockDim.x * blockIdx.x + threadIdx.x;
  int cacheindex = threadIdx.x;
  int n = data.size().x * data.size().y;

  T temp = -1 << 20;
  while (tid < n) {
    int i = tid / data.size().x;
    int j = tid % data.size().x;
    temp = fmaxf(temp, data(i, j));
    tid += blockDim.x * gridDim.x;
  }
  cache[cacheindex] = temp;
  __syncthreads();

  int i = blockDim.x / 2;
  while (i != 0) {
    if (cacheindex < i)
      cache[cacheindex] = fmaxf(cache[cacheindex], cache[cacheindex + i]);
    __syncthreads();
    i /= 2;
  }
  if (cacheindex == 0)
    c[blockIdx.x] = cache[0];
}

template <typename T> T maxValue(MemoryBlock2<MemoryLocation::DEVICE, T> &data) {
  size_t blockSize = (data.size().x * data.size().y + 256 - 1) / 256;
  if (blockSize > 32)
    blockSize = 32;
  T *c = new T[blockSize];
  T *d_c;
  cudaMalloc((void **)&d_c, blockSize * sizeof(T));
  k__max<<<blockSize, 256>>>(data.accessor(), d_c);
  cudaMemcpy(c, d_c, blockSize * sizeof(T), cudaMemcpyDeviceToHost);
  T norm = 0;
  for (int i = 0; i < blockSize; i++)
    norm = fmax(norm, c[i]);
  cudaFree(d_c);
  delete[] c;
  return norm;
}

template <typename T>
__global__ void __max_abs(MemoryBlock2Accessor<T> data, T *c) {
  __shared__ float cache[256];

  int tid = blockDim.x * blockIdx.x + threadIdx.x;
  int cacheindex = threadIdx.x;
  int n = data.size().x * data.size().y;

  T temp = -1 << 20;
  while (tid < n) {
    int i = tid / data.size().x;
    int j = tid % data.size().x;
    temp = fmaxf(temp, fabsf(data(i, j)));
    tid += blockDim.x * gridDim.x;
  }
  cache[cacheindex] = temp;
  __syncthreads();

  int i = blockDim.x / 2;
  while (i != 0) {
    if (cacheindex < i)
      cache[cacheindex] = fmaxf(cache[cacheindex], cache[cacheindex + i]);
    __syncthreads();
    i /= 2;
  }
  if (cacheindex == 0)
    c[blockIdx.x] = cache[0];
}

template <typename T> T maxAbs(MemoryBlock2<MemoryLocation::DEVICE, T> &data) {
  size_t blockSize = (data.size().x * data.size().y + 256 - 1) / 256;
  if (blockSize > 32)
    blockSize = 32;
  T *c = new T[blockSize];
  T *d_c;
  cudaMalloc((void **)&d_c, blockSize * sizeof(T));
  __max_abs<<<blockSize, 256>>>(data.accessor(), d_c);
  cudaMemcpy(c, d_c, blockSize * sizeof(T), cudaMemcpyDeviceToHost);
  T norm = 0;
  for (int i = 0; i < blockSize; i++)
    norm = fmax(norm, c[i]);
  cudaFree(d_c);
  delete[] c;
  return norm;
}

template <typename T>
__global__ void __max_abs(MemoryBlock3Accessor<T> data, T *c) {
  __shared__ float cache[256];

  int tid = blockDim.x * blockIdx.x + threadIdx.x;
  int cacheindex = threadIdx.x;
  int n = data.size().x * data.size().y * data.size().z;

  T temp = -1 << 20;
  while (tid < n) {
    int k = tid / (data.size().x * data.size().y);
    int j = (tid % (data.size().x * data.size().y)) / data.size().x;
    int i = tid % data.size().x;
    temp = fmaxf(temp, fabsf(data(i, j, k)));
    tid += blockDim.x * gridDim.x;
  }
  cache[cacheindex] = temp;
  __syncthreads();

  int i = blockDim.x / 2;
  while (i != 0) {
    if (cacheindex < i)
      cache[cacheindex] = fmaxf(cache[cacheindex], cache[cacheindex + i]);
    __syncthreads();
    i /= 2;
  }
  if (cacheindex == 0)
    c[blockIdx.x] = cache[0];
}

template <typename T> T maxAbs(MemoryBlock3<MemoryLocation::DEVICE, T> &data) {
  size_t blockSize =
      (data.size().x * data.size().y * data.size().z + 256 - 1) / 256;
  if (blockSize > 32)
    blockSize = 32;
  T *c = new T[blockSize];
  T *d_c;
  cudaMalloc((void **)&d_c, blockSize * sizeof(T));
  __max_abs<<<blockSize, 256>>>(data.accessor(), d_c);
  cudaMemcpy(c, d_c, blockSize * sizeof(T), cudaMemcpyDeviceToHost);
  T norm = 0;
  for (int i = 0; i < blockSize; i++)
    norm = fmax(norm, c[i]);
  cudaFree(d_c);
  delete[] c;
  return norm;
}

} // namespace cuda

} // namespace hermes

#endif