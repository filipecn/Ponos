/// Copyright (c) 2020, FilipeCN.
///
/// The MIT License (MIT)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///\file array.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2020-01-28
///
///\brief

#ifndef HERMES_STORAGE_ARRAY_H
#define HERMES_STORAGE_ARRAY_H

#include <hermes/common/cuda.h>
#include <hermes/common/index.h>
#include <ponos/storage/array.h>

namespace hermes {

namespace cuda {

/*****************************************************************************
*************************           ARRAY1           *************************
******************************************************************************/

///\brief
///
///\tparam T
template <typename T> class Array1Accessor {
public:
  Array1Accessor(T *data, size_t size) : size_(size), data_(data) {}
  __host__ __device__ size_t size() const { return size_; }
  __device__ T &operator[](size_t i) { return data_[i]; }
  __device__ const T &operator[](size_t i) const { return data_[i]; }
  __device__ bool contains(int i) const { return i >= 0 && i < (int)size_; }

protected:
  size_t size_;
  T *data_ = nullptr;
};

template <typename T> class Array1CAccessor {
public:
  Array1CAccessor(const T *data, size_t size) : size_(size), data_(data) {}
  __host__ __device__ size_t size() const { return size_; }
  __device__ const T &operator[](size_t i) const { return data_[i]; }
  __device__ bool contains(int i) const { return i >= 0 && i < (int)size_; }

protected:
  size_t size_;
  const T *data_ = nullptr;
};

template <typename T> __global__ void __fill(Array1Accessor<T> array, T value) {
  int x = blockIdx.x * blockDim.x + threadIdx.x;
  if (array.contains(x))
    array[x] = value;
}

template <typename T> void fill(Array1Accessor<T> array, T value) {
  ThreadArrayDistributionInfo td(array.size());
  __fill<T><<<td.gridSize, td.blockSize>>>(array, value);
}

/// Holds a linear memory area representing a 1-dimensional array.
/// \tparam T data type
template <typename T> class Array1 {
public:
  Array1() = default;
  Array1(size_t size) : size_(size) { resize(size); }
  Array1(size_t size, T value) : size_(size) {
    resize(size);
    auto acc = Array1Accessor<T>((T *)data_, size_);
    fill(acc, value);
  }
  Array1(const Array1 &other) : Array1(other.size_) {
    CHECK_CUDA(cudaMemcpy(data_, other.data_, size_ * sizeof(T),
                          cudaMemcpyDeviceToDevice));
  }
  Array1(const Array1 &&other) = delete;
  Array1(Array1 &&other) noexcept : size_(other.size_), data_(other.data_) {
    other.size_ = 0;
    other.data_ = nullptr;
  }
  Array1(const std::vector<T> &data) {
    resize(data.size());
    CHECK_CUDA(
        cudaMemcpy(data_, &data[0], size_ * sizeof(T), cudaMemcpyHostToDevice));
  }
  ~Array1() { clear(); }
  ///\param other **[in]**
  ///\return Array1<T>&
  Array1<T> &operator=(const Array1<T> &other) {
    resize(other.size_);
    CHECK_CUDA(cudaMemcpy(data_, other.data_, size_ * sizeof(T),
                          cudaMemcpyDeviceToDevice));
    return *this;
  }
  ///\param data **[in]**
  ///\return Array1<T>&
  Array1<T> &operator=(const std::vector<T> &data) {
    resize(data.size());
    CHECK_CUDA(
        cudaMemcpy(data_, &data[0], size_ * sizeof(T), cudaMemcpyHostToDevice));
    return *this;
  }
  Array1<T> &operator=(T value) {
    fill(Array1Accessor<T>((T *)data_, size_), value);
    return *this;
  }
  /// frees any previous data and allocates a new block
  ///\param new_size **[in]** new element count
  void resize(size_t new_size) {
    if (data_)
      clear();
    size_ = new_size;
    CHECK_CUDA(cudaMalloc(&data_, size_ * sizeof(T)));
  }
  /// frees memory and set size to zero
  void clear() {
    if (data_)
      CHECK_CUDA(cudaFree(data_));
    data_ = nullptr;
    size_ = 0;
  }
  ///\return size_t memory block size in elements
  size_t size() const { return size_; }
  ///\return size_t memory block size in bytes
  size_t memorySize() const { return size_ * sizeof(T); }
  ///\return const T* device pointer
  const T *data() const { return (const T *)data_; }
  ///\return  T* device pointer
  T *data() { return (T *)data_; }
  /// copies data to host side
  ///\return std::vector<T> data in host side
  std::vector<T> hostData() const {
    std::vector<T> data(size_);
    CHECK_CUDA(
        cudaMemcpy(&data[0], data_, size_ * sizeof(T), cudaMemcpyDeviceToHost));
    return data;
  }
  Array1Accessor<T> accessor() { return Array1Accessor<T>((T *)data_, size_); }
  Array1CAccessor<T> constAccessor() const {
    return Array1CAccessor<T>((const T *)data_, size_);
  }

private:
  size_t size_{0};
  void *data_{nullptr};
};

using array1d = Array1<double>;
using array1f = Array1<float>;
using array1i = Array1<int>;
using array1u = Array1<unsigned int>;

/*****************************************************************************
*************************       ARRAY2 ACCESSOR      *************************
******************************************************************************/
///\tparam T array data type
template <typename T> class Array2Accessor {
public:
  Array2Accessor(T *data, size2 size, size_t pitch)
      : size_(size), pitch_(pitch), data_(data) {}
  __host__ __device__ size2 size() const { return size_; }
  __device__ T &operator[](index2 ij) {
    return (T &)(*((char *)data_ + ij.j * pitch_ + ij.i * sizeof(T)));
  }
  __device__ const T &operator[](index2 ij) const {
    return (
        const T &)(*((const char *)data_ + ij.j * pitch_ + ij.i * sizeof(T)));
  }
  __device__ bool contains(index2 ij) const {
    return ij >= index2() && ij < size_;
  }

protected:
  size2 size_;
  size_t pitch_;
  T *data_ = nullptr;
};
/*****************************************************************************
*********************     ARRAY2 CONST ACCESSOR      *************************
******************************************************************************/
///\tparam T array data type
template <typename T> class Array2CAccessor {
public:
  Array2CAccessor(const T *data, size2 size, size_t pitch)
      : size_(size), pitch_(pitch), data_(data) {}
  __host__ __device__ size2 size() const { return size_; }
  __device__ const T &operator[](index2 ij) const {
    return (
        const T &)(*((const char *)data_ + ij.j * pitch_ + ij.i * sizeof(T)));
  }
  __device__ bool contains(index2 ij) const {
    return ij >= index2() && ij < size_;
  }

protected:
  size2 size_;
  size_t pitch_;
  const T *data_ = nullptr;
};
/*****************************************************************************
**************************         ARRAY2            *************************
******************************************************************************/
template <typename T> __global__ void __fill(Array2Accessor<T> array, T value) {
  index2 index(blockIdx.x * blockDim.x + threadIdx.x,
               blockIdx.y * blockDim.y + threadIdx.y);
  if (array.contains(index))
    array[index] = value;
}
/// Ste all values of the array2 to **value**
///\tparam T array data type
///\param array **[in]**
///\param value **[in]**
template <typename T> void fill(Array2Accessor<T> array, T value) {
  ThreadArrayDistributionInfo td(array.size());
  __fill<T><<<td.gridSize, td.blockSize>>>(array, value);
}
/// Auxiliary class that encapsulates a c++ lambda function into device code
///\tparam T array data type
///\tparam F lambda function type following the signature: (index2, T&)
template <typename T, typename F> struct map_operation {
  __host__ __device__ explicit map_operation(const F &op) : operation(op) {}
  __host__ __device__ void operator()(index2 index, T &value) {
    operation(index, value);
  }
  F operation;
};
///\brief
///
///\tparam T
///\tparam F
///\param array **[in]**
///\param operation **[in]**
template <typename T, typename F>
__global__ void __map(Array2Accessor<T> array, map_operation<T, F> operation) {
  index2 index(blockIdx.x * blockDim.x + threadIdx.x,
               blockIdx.y * blockDim.y + threadIdx.y);
  if (array.contains(index))
    operation(index, array[index]);
}
/// Apply **operation** to all elements of **array**
///\tparam T array data type
///\tparam F lambda function type
///\param array **[in]**
///\param operation **[in]** lambda function with the following signature:
///(index2, T&)
template <typename T, typename F>
void __host__ __device__ mapArray(Array2Accessor<T> array,
                                  map_operation<T, F> operation) {
  ThreadArrayDistributionInfo td(array.size());
  __map<T, F><<<td.gridSize, td.blockSize>>>(array, operation);
}

/// Holds a linear memory area representing a 1-dimensional array.
/// \tparam T data type
template <typename T> class Array2 {
public:
  // ***********************************************************************
  //                           CONSTRUCTORS
  // ***********************************************************************
  Array2() = default;
  ///\param size **[in]**
  Array2(size2 size) : size_(size) { resize(size); }
  ///\param size **[in]**
  ///\param value **[in]**
  Array2(size2 size, T value) : size_(size) {
    resize(size);
    auto acc = Array2Accessor<T>((T *)data_, size_, pitch_);
    fill(acc, value);
  }
  Array2(const Array2<T> &other) = delete;
  ///\param other **[in]**
  Array2(Array2<T> &other) {
    resize(other.size_);
    copyPitchedToPitched<T>(pitchedData(), other.pitchedData(),
                            cudaMemcpyDeviceToDevice);
  }
  Array2(const Array2 &&other) = delete;
  ///\param other **[in]**
  Array2(Array2 &&other) noexcept
      : size_(other.size_), data_(other.data_), pitch_(other.pitch_) {
    other.size_ = size2(0, 0);
    other.pitch_ = 0;
    other.data_ = nullptr;
  }
  Array2(const ponos::Array2<T> &data) = delete;
  ///\param data **[in]**
  Array2(ponos::Array2<T> &data) {
    resize(data.size());
    copyPitchedToPitched<T>(pitchedData(), data.pitchedData(),
                            cudaMemcpyHostToDevice);
  }
  ~Array2() { clear(); }
  // ***********************************************************************
  //                            OPERATORS
  // ***********************************************************************
  ///\param other **[in]**
  ///\return Array2<T>&
  Array2<T> &operator=(const Array2<T> &other) = delete;
  ///\param other **[in]**
  ///\return Array1<T>&
  Array2<T> &operator=(Array2<T> &other) {
    resize(other.size_);
    copyPitchedToPitched<T>(pitchedData(), other.pitchedData(),
                            cudaMemcpyDeviceToDevice);
    return *this;
  }
  Array2<T> &operator=(const ponos::Array2<T> &data) = delete;
  Array2<T> &operator=(const ponos::Array2<T> &&data) = delete;
  ///\param data **[in]**
  ///\return Array1<T>&
  Array2<T> &operator=(ponos::Array2<T> &data) {
    resize(size2(data.size().width, data.size().height));
    copyPitchedToPitched<T>(pitchedData(), pitchedDataFrom(data),
                            cudaMemcpyHostToDevice);
    return *this;
  }
  /// \param data **[in]**
  /// \return Array2<T>&
  Array2<T> &operator=(ponos::Array2<T> &&data) {
    resize(size2(data.size().width, data.size().height));
    copyPitchedToPitched<T>(pitchedData(), pitchedData(data),
                            cudaMemcpyHostToDevice);
    return *this;
  }
  /// \param value **[in]**
  /// \return Array2<T>&
  Array2<T> &operator=(T value) {
    fill(Array2Accessor<T>((T *)data_, size_, pitch_), value);
    return *this;
  }
  // ***********************************************************************
  //                         GETTERS & SETTERS
  // ***********************************************************************
  /// frees any previous data and allocates a new block
  ///\param new_size **[in]** new element count
  void resize(size2 new_size) {
    if (data_)
      clear();
    size_ = new_size;
    if (size_.total() == 0) {
      clear();
      return;
    }
    void *pdata = nullptr;
    CHECK_CUDA(cudaMallocPitch(&pdata, &pitch_, size_.width * sizeof(T),
                               size_.height));
    data_ = reinterpret_cast<T *>(pdata);
  }
  ///\return size_t memory block size in elements
  size2 size() const { return size_; }
  ///\return u32
  size_t pitch() const { return pitch_; }
  ///\return size_t memory block size in bytes
  size_t memorySize() const { return pitch_ * size_.height; }
  ///\return const T* raw device pointer
  const T *data() const { return (const T *)data_; }
  ///\return  T* raw device pointer
  T *data() { return (T *)data_; }
  ///\return cudaPitchedPtr
  cudaPitchedPtr pitchedData() {
    cudaPitchedPtr pd{0};
    pd.ptr = data_;
    pd.pitch = pitch_;
    pd.xsize = size_.width * sizeof(T);
    pd.ysize = size_.height;
    return pd;
  }
  /// copies data to host side
  ///\return std::vector<T> data in host side
  ponos::Array2<T> hostData() {
    ponos::Array2<T> data(ponos::size2(size_.width, size_.height), pitch_);
    copyPitchedToPitched<T>(pitchedDataFrom(data), pitchedData(),
                            cudaMemcpyDeviceToHost);
    return data;
  }
  Array2Accessor<T> accessor() {
    return Array2Accessor<T>((T *)data_, size_, pitch_);
  }
  Array2CAccessor<T> constAccessor() const {
    return Array2CAccessor<T>((const T *)data_, size_, pitch_);
  }
  // ***********************************************************************
  //                            METHODS
  // ***********************************************************************
  /// frees memory and set size to zero
  void clear() {
    if (data_)
      CHECK_CUDA(cudaFree(data_));
    data_ = (T *)nullptr;
    size_ = size2(0, 0);
  }
  ///\tparam F function type
  ///\param operation **[in]**
  template <typename F> void map(F operation) {
    mapArray<T, F>(Array2Accessor<T>((T *)data_, size_, pitch_),
                   map_operation<T, F>(operation));
  }

private:
  size2 size_{0};
  size_t pitch_ = 0;
  void *data_{nullptr};
};

using array2d = Array2<double>;
using array2f = Array2<float>;
using array2i = Array2<int>;
using array2u = Array2<unsigned int>;
} // namespace cuda

} // namespace hermes

#endif