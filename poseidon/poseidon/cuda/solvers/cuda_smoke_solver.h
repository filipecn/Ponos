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

#ifndef POSEIDON_SOLVERS_CUDA_SMOKE_SOLVER_H
#define POSEIDON_SOLVERS_CUDA_SMOKE_SOLVER_H

#include <hermes/storage/cuda_storage_utils.h>
#include <memory>
#include <ponos/geometry/point.h>
#include <poseidon/math/cuda_fd.h>
#include <poseidon/simulation/cuda_integrator.h>
#include <poseidon/simulation/cuda_scene.h>
#include <poseidon/solvers/cuda_smoke_solver2_steps.h>
#include <poseidon/solvers/cuda_smoke_solver3_kernels.h>
#include <poseidon/solvers/cuda_smoke_solver_kernels.h>

namespace poseidon {

namespace cuda {

inline __global__ void __setupScene(poseidon::cuda::Collider2<float> **solids,
                                    poseidon::cuda::Collider2<float> **scene) {
  if (threadIdx.x == 0 && blockIdx.x == 0) {
    float d = 1.0 / 64;
    // floor
    solids[0] = new poseidon::cuda::BoxCollider2<float>(hermes::cuda::bbox2(
        hermes::cuda::point2(0.f, 0.f), hermes::cuda::point2(1.f, d)));
    // ceil
    solids[1] = new poseidon::cuda::BoxCollider2<float>(hermes::cuda::bbox2(
        hermes::cuda::point2(0.f, 1.f - d), hermes::cuda::point2(1.f, 1.f)));
    // left
    solids[2] = new poseidon::cuda::BoxCollider2<float>(hermes::cuda::bbox2(
        hermes::cuda::point2(0.f, 0.f), hermes::cuda::point2(d, 1.f)));
    // right
    solids[3] = new poseidon::cuda::BoxCollider2<float>(hermes::cuda::bbox2(
        hermes::cuda::point2(1.f - d, 0.f), hermes::cuda::point2(1.f, 1.f)));
    solids[4] = new poseidon::cuda::SphereCollider2<float>(
        hermes::cuda::point2(0.5f, 0.5f), 0.1f);
    *scene = new poseidon::cuda::Collider2Set<float>(solids, 4);
  }
}

inline __global__ void __setupScene(poseidon::cuda::Collider2<float> **solids,
                                    poseidon::cuda::Collider2<float> **scene,
                                    int res) {
  if (threadIdx.x == 0 && blockIdx.x == 0) {
    float d = 1.0 / res;
    // floor
    solids[0] = new poseidon::cuda::BoxCollider2<float>(hermes::cuda::bbox2(
        hermes::cuda::point2(0.f), hermes::cuda::point2(1.f, d)));
    // ceil
    solids[1] = new poseidon::cuda::BoxCollider2<float>(hermes::cuda::bbox2(
        hermes::cuda::point2(0.f, 1.f - d), hermes::cuda::point2(1.f)));
    // left
    solids[2] = new poseidon::cuda::BoxCollider2<float>(hermes::cuda::bbox2(
        hermes::cuda::point2(0.f), hermes::cuda::point2(d, 1.f)));
    // right
    solids[2] = new poseidon::cuda::BoxCollider2<float>(hermes::cuda::bbox2(
        hermes::cuda::point2(1.f - d, 0.f), hermes::cuda::point2(1.f)));
    *scene = new poseidon::cuda::Collider2Set<float>(solids, 4);
  }
}

inline __global__ void __setupScene(poseidon::cuda::Collider3<float> **solids,
                                    poseidon::cuda::Collider3<float> **scene,
                                    int res) {
  if (threadIdx.x == 0 && blockIdx.x == 0) {
    float d = 1.0 / res;
    // floor
    solids[0] = new poseidon::cuda::BoxCollider3<float>(hermes::cuda::bbox3(
        hermes::cuda::point3(0.f), hermes::cuda::point3(1.f, d, 1.f)));
    // ceil
    solids[1] = new poseidon::cuda::BoxCollider3<float>(hermes::cuda::bbox3(
        hermes::cuda::point3(0.f, 1.f - d, 0.f), hermes::cuda::point3(1.f)));
    // left
    solids[2] = new poseidon::cuda::BoxCollider3<float>(hermes::cuda::bbox3(
        hermes::cuda::point3(0.f), hermes::cuda::point3(d, 1.f, 1.f)));
    // right
    solids[3] = new poseidon::cuda::BoxCollider3<float>(hermes::cuda::bbox3(
        hermes::cuda::point3(1.f - d, 0.f, 0.f), hermes::cuda::point3(1.f)));
    // back
    solids[4] = new poseidon::cuda::BoxCollider3<float>(hermes::cuda::bbox3(
        hermes::cuda::point3(0.f), hermes::cuda::point3(1.f, 1.f, d)));
    // front
    solids[5] = new poseidon::cuda::BoxCollider3<float>(hermes::cuda::bbox3(
        hermes::cuda::point3(0.f, 0.f, 1.f - d), hermes::cuda::point3(1.f)));
    *scene = new poseidon::cuda::Collider3Set<float>(solids, 6);
  }
}

inline __global__ void __freeScene(poseidon::cuda::Collider2<float> **solids) {
  if (threadIdx.x == 0 && blockIdx.x == 0)
    for (int i = 0; i < 5; ++i)
      delete solids[i];
}

inline __global__ void __freeScene(poseidon::cuda::Collider3<float> **solids) {
  if (threadIdx.x == 0 && blockIdx.x == 0)
    for (int i = 0; i < 5; ++i)
      delete solids[i];
}

inline __global__ void __rasterColliders(Collider2<float> *const *colliders,
                                         unsigned char *solids, float *u,
                                         float *v,
                                         hermes::cuda::Grid2Info sInfo) {
  int x = blockIdx.x * blockDim.x + threadIdx.x;
  int y = blockIdx.y * blockDim.y + threadIdx.y;
  int index = y * sInfo.resolution.x + x;
  if (x < sInfo.resolution.x && y < sInfo.resolution.y) {
    if ((*colliders)->intersect(sInfo.toWorld(hermes::cuda::point2(x, y))))
      solids[index] = 1;
    else
      solids[index] = 0;
    u[y * sInfo.resolution.x + x] = u[y * sInfo.resolution.x + x + 1] = 0;
    v[y * sInfo.resolution.x + x] = v[(y + 1) * sInfo.resolution.x + x] = 0;
  }
}

inline __global__ void
__rasterColliders(Collider2<float> *const *colliders,
                  hermes::cuda::RegularGrid2Accessor<unsigned char> solid,
                  hermes::cuda::RegularGrid2Accessor<float> u,
                  hermes::cuda::RegularGrid2Accessor<float> v) {
  int x = blockIdx.x * blockDim.x + threadIdx.x;
  int y = blockIdx.y * blockDim.y + threadIdx.y;
  if (solid.isIndexStored(x, y)) {
    using namespace hermes::cuda;
    if ((*colliders)->intersect(solid.worldPosition(x, y)))
      solid(x, y) = 1;
    else
      solid(x, y) = 0;
    u(x, y) = u(x + 1, y) = 0;
    v(x, y) = v(x, y + 1) = 0;
  }
}

inline __global__ void
__rasterColliders(Collider3<float> *const *colliders,
                  hermes::cuda::RegularGrid3Accessor<unsigned char> solid,
                  hermes::cuda::RegularGrid3Accessor<float> u,
                  hermes::cuda::RegularGrid3Accessor<float> v,
                  hermes::cuda::RegularGrid3Accessor<float> w) {
  int x = blockIdx.x * blockDim.x + threadIdx.x;
  int y = blockIdx.y * blockDim.y + threadIdx.y;
  int z = blockIdx.z * blockDim.z + threadIdx.z;
  if (solid.isIndexStored(x, y, z)) {
    using namespace hermes::cuda;
    if ((*colliders)->intersect(solid.worldPosition(x, y, z)))
      solid(x, y, z) = 1;
    else
      solid(x, y, z) = 0;
    u(x, y, z) = u(x + 1, y, z) = 0;
    v(x, y, z) = v(x, y + 1, z) = 0;
    w(x, y, z) = w(x, y, z + 1) = 0;
  }
}

inline __global__ void __normalizeIFFT(float *g_data, int width, int height,
                                       float N) {

  // index = x * height + y

  unsigned int xIndex = blockDim.x * blockIdx.x + threadIdx.x;

  unsigned int yIndex = blockDim.y * blockIdx.y + threadIdx.y;

  int index = yIndex * width + xIndex;

  g_data[index] = g_data[index] / N;
}

/// Eulerian grid based solver for smoke simulations. Stores its data in fast
/// device texture memory.
template <typename GridType> class GridSmokeSolver2_t {
public:
  GridSmokeSolver2_t() = default;
  ~GridSmokeSolver2_t() {
    unbindTextures();
    __freeScene<<<1, 1>>>(scene.list);
    using namespace hermes::cuda;
    CUDA_CHECK(cudaFree(scene.list));
    CUDA_CHECK(cudaFree(scene.colliders));
  }
  void setUIntegrator(Integrator2 *integrator) {
    uIntegrator.reset(integrator);
  }
  void setVIntegrator(Integrator2 *integrator) {
    vIntegrator.reset(integrator);
  }
  void setIntegrator(Integrator2 *_integrator) {
    integrator.reset(_integrator);
  }
  void init() {
    setupTextures();
    bindTextures(velocity, velocityCopy, scalarFields[0], divergence, pressure,
                 solid, forceField, solidVelocity);
    using namespace hermes::cuda;
    CUDA_CHECK(cudaMalloc(&scene.list, 5 * sizeof(Collider2<float> *)));
    CUDA_CHECK(cudaMalloc(&scene.colliders, sizeof(Collider2<float> *)));
    // create fft plans
    auto uRes = velocity.u().resolution();
    auto vRes = velocity.v().resolution();
    if (cufftPlan2d(&forwardPlanU, uRes.x, uRes.y, CUFFT_R2C) !=
        CUFFT_SUCCESS) {
      std::cerr << "CUFFT Error: Failed to create plan\n";
      exit(-1);
    }
    if (cufftPlan2d(&inversePlanU, uRes.x, uRes.y, CUFFT_C2R) !=
        CUFFT_SUCCESS) {
      std::cerr << "CUFFT Error: Failed to create plan\n";
      exit(-1);
    }
    if (cufftPlan2d(&forwardPlanV, vRes.x, vRes.y, CUFFT_R2C) !=
        CUFFT_SUCCESS) {
      std::cerr << "CUFFT Error: Failed to create plan\n";
      exit(-1);
    }
    if (cufftPlan2d(&inversePlanV, vRes.x, vRes.y, CUFFT_C2R) !=
        CUFFT_SUCCESS) {
      std::cerr << "CUFFT Error: Failed to create plan\n";
      exit(-1);
    }
    // allocate device memory for frequency space
    CUDA_CHECK(cudaMalloc((void **)&d_frequenciesU,
                          sizeof(cufftComplex) * uRes.x * (uRes.y / 2 + 1)));
    CUDA_CHECK(cudaMalloc((void **)&d_frequenciesV,
                          sizeof(cufftComplex) * vRes.x * (vRes.y / 2 + 1)));
  }
  ///
  /// \param res
  void setResolution(const ponos::size2 &res) {
    resolution = hermes::cuda::vec2u(res.width, res.height);
    velocity.resize(resolution);
    velocityCopy.resize(resolution);
    for (auto &f : scalarFields)
      f.resize(resolution);
    pressure.resize(resolution);
    divergence.resize(resolution);
    solid.resize(resolution);
    solidVelocity.resize(resolution);
    forceField.resize(resolution);
    if (scalarFields.size())
      integrator->set(scalarFields[0].info());
    uIntegrator->set(velocity.u().info());
    vIntegrator->set(velocity.v().info());
  }
  /// Sets cell size
  /// \param _dx scale
  void setDx(float _dx) {
    dx = _dx;
    velocity.setDx(dx);
    velocityCopy.setDx(dx);
    for (auto &f : scalarFields)
      f.setDx(dx);
    pressure.setDx(dx);
    divergence.setDx(dx);
    solid.setDx(dx);
    solidVelocity.setDx(dx);
    forceField.setDx(dx);
    if (scalarFields.size())
      integrator->set(scalarFields[0].info());
    uIntegrator->set(velocity.u().info());
    vIntegrator->set(velocity.v().info());
  }
  /// Sets lower left corner position
  /// \param o offset
  void setOrigin(const ponos::point2f &o) {
    hermes::cuda::point2f p(o.x, o.y);
    velocity.setOrigin(p);
    velocityCopy.setOrigin(p);
    for (auto &f : scalarFields)
      f.setOrigin(p);
    pressure.setOrigin(p);
    divergence.setOrigin(p);
    solid.setOrigin(p);
    solidVelocity.setOrigin(p);
    forceField.setOrigin(p);
    if (scalarFields.size())
      integrator->set(scalarFields[0].info());
    uIntegrator->set(velocity.u().info());
    vIntegrator->set(velocity.v().info());
  }
  size_t addScalarField() {
    scalarFields.emplace_back();
    return scalarFields.size() - 1;
  }
  /// Advances one simulation step
  /// \param dt time step
  void step(float dt) {
    using namespace hermes::cuda;
    // rasterColliders();
    // CUDA_CHECK(cudaDeviceSynchronize());
    uIntegrator->advect(velocity, solid, velocity.u(), velocity.u(), dt);
    CUDA_CHECK(cudaDeviceSynchronize());
    vIntegrator->advect(velocity, solid, velocity.v(), velocity.v(), dt);
    CUDA_CHECK(cudaDeviceSynchronize());
    velocity.u().texture().updateTextureMemory();
    velocity.v().texture().updateTextureMemory();
    for (size_t i = 0; i < scalarFields.size(); i++)
      integrator->advect(velocity, solid, scalarFields[i], scalarFields[i], dt);
    CUDA_CHECK(cudaDeviceSynchronize());
    applyForceField(velocity, forceField, dt);
    // velocityCopy.copy(velocity);
    // velocityCopy.u().texture().updateTextureMemory();
    // velocityCopy.v().texture().updateTextureMemory();
    // diffuse(velocity, 0.4, dt);
    CUDA_CHECK(cudaDeviceSynchronize());
    computeDivergence(velocity, solid, divergence);
    CUDA_CHECK(cudaDeviceSynchronize());
    computePressure(divergence, solid, pressure, dt, 128);
    CUDA_CHECK(cudaDeviceSynchronize());
    projectionStep(pressure, solid, velocity, dt);
    CUDA_CHECK(cudaDeviceSynchronize());
    // hermes::cuda::fill(forceField.u().texture(), 0.0f);
    // hermes::cuda::fill(forceField.v().texture(), 0.0f);
  }
  /// Advances one simulation step folllowing original Stan method
  /// \param dt time step
  void stepFFT(float dt) {
    using namespace hermes::cuda;
    applyForceField(velocity, forceField, dt);
    hermes::cuda::fill(forceField.u().texture(), 0.0f);
    hermes::cuda::fill(forceField.v().texture(), 0.0f);
    CUDA_CHECK(cudaDeviceSynchronize());
    uIntegrator->advect(velocity, solid, velocity.u(), velocity.u(), dt);
    CUDA_CHECK(cudaDeviceSynchronize());
    vIntegrator->advect(velocity, solid, velocity.v(), velocity.v(), dt);
    CUDA_CHECK(cudaDeviceSynchronize());
    velocity.u().texture().updateTextureMemory();
    velocity.v().texture().updateTextureMemory();
    for (size_t i = 0; i < scalarFields.size(); i++)
      integrator->advect(velocity, solid, scalarFields[i], scalarFields[i], dt);
    CUDA_CHECK(cudaDeviceSynchronize());
    applyFFT();
    diffuseFFT(resolution, d_frequenciesU, d_frequenciesV, 0.001, dt);
    projectFFT(resolution, d_frequenciesU, d_frequenciesV);
    applyiFFT();
  }
  /// Raster collider bodies and velocities into grid simulations
  void rasterColliders() {
    __setupScene<<<1, 1>>>(scene.list, scene.colliders);
    hermes::ThreadArrayDistributionInfo td(resolution);
    __rasterColliders<<<td.gridSize, td.blockSize>>>(
        scene.colliders, solid.texture().deviceData(),
        solidVelocity.uDeviceData(), solidVelocity.vDeviceData(), solid.info());
    solid.texture().updateTextureMemory();
    solidVelocity.u().texture().updateTextureMemory();
    solidVelocity.v().texture().updateTextureMemory();
  }
  /// \return hermes::cuda::StaggeredGridTexture2&
  GridType &velocityData() { return velocity; }
  /// \return const hermes::cuda::GridTexture2<float>& density data reference
  const hermes::cuda::GridTexture2<float> &pressureData() const {
    return pressure;
  }
  const hermes::cuda::GridTexture2<float> &scalarField(size_t i) const {
    return scalarFields[i];
  }
  hermes::cuda::GridTexture2<float> &scalarField(size_t i) {
    return scalarFields[i];
  }
  /// \return const hermes::cuda::GridTexture2<char>&
  const hermes::cuda::GridTexture2<unsigned char> &solidData() const {
    return solid;
  }
  /// \return  hermes::cuda::GridTexture2<char>
  hermes::cuda::GridTexture2<unsigned char> &solidData() { return solid; }
  /// \return hermes::cuda::StaggeredGridTexture2&
  const GridType &solidVelocityData() const { return solidVelocity; }
  /// \return hermes::cuda::StaggeredGridTexture2&
  GridType &solidVelocityData() { return solidVelocity; }
  /// \return hermes::cuda::StaggeredGridTexture2&
  const GridType &forceFieldData() const { return forceField; }
  /// \return hermes::cuda::StaggeredGridTexture2&
  GridType &forceFieldData() { return forceField; }

  Scene2<float> scene;

protected:
  void applyFFT() {
    using namespace hermes::cuda;
    if (cufftExecR2C(forwardPlanU, velocity.uDeviceData(), d_frequenciesU) !=
        CUFFT_SUCCESS) {
      fprintf(stderr, "CUFFT Error: Unable to execute plan\n");
      exit(-1);
    }
    CUDA_CHECK(cudaDeviceSynchronize());
    if (cufftExecR2C(forwardPlanV, velocity.vDeviceData(), d_frequenciesV) !=
        CUFFT_SUCCESS) {
      fprintf(stderr, "CUFFT Error: Unable to execute plan\n");
      exit(-1);
    }
    CUDA_CHECK(cudaDeviceSynchronize());
  }

  void applyiFFT() {
    using namespace hermes::cuda;
    if (cufftExecC2R(inversePlanU, d_frequenciesU, velocity.uDeviceData()) !=
        CUFFT_SUCCESS) {
      fprintf(stderr, "CUFFT Error: Unable to execute plan\n");
      exit(-1);
    }
    CUDA_CHECK(cudaDeviceSynchronize());
    {
      dim3 grid(resolution.x / 16, resolution.y / 16, 1);
      dim3 threads(16, 16, 1);
      __normalizeIFFT<<<grid, threads>>>(velocity.uDeviceData(), resolution.x,
                                         resolution.y,
                                         resolution.x * resolution.y);
    }
    CUDA_CHECK(cudaDeviceSynchronize());
    if (cufftExecC2R(inversePlanV, d_frequenciesV, velocity.vDeviceData()) !=
        CUFFT_SUCCESS) {
      fprintf(stderr, "CUFFT Error: Unable to execute plan\n");
      exit(-1);
    }
    CUDA_CHECK(cudaDeviceSynchronize());
    {
      dim3 grid(resolution.x / 16, resolution.y / 16, 1);
      dim3 threads(16, 16, 1);
      __normalizeIFFT<<<grid, threads>>>(velocity.vDeviceData(), resolution.x,
                                         resolution.y,
                                         resolution.x * resolution.y);
    }
    CUDA_CHECK(cudaDeviceSynchronize());
    velocity.u().texture().updateTextureMemory();
    velocity.v().texture().updateTextureMemory();
  }

  std::shared_ptr<Integrator2> vIntegrator;
  std::shared_ptr<Integrator2> uIntegrator;
  std::shared_ptr<Integrator2> integrator;
  GridType velocity, velocityCopy;
  GridType solidVelocity;
  GridType forceField;
  hermes::cuda::GridTexture2<float> pressure;
  hermes::cuda::GridTexture2<float> divergence;
  hermes::cuda::GridTexture2<unsigned char> solid;
  std::vector<hermes::cuda::GridTexture2<float>> scalarFields;
  hermes::cuda::vec2u resolution;
  float dx = 1;
  // fft
  cufftComplex *d_frequenciesU, *d_frequenciesV;
  cufftHandle forwardPlanU, inversePlanU;
  cufftHandle forwardPlanV, inversePlanV;
};

/// Eulerian grid based solver for smoke simulations. Stores its data in fast
/// device texture memory.
class GridSmokeSolver2 {
public:
  GridSmokeSolver2() {
    uIntegrator_.reset(new MacCormackIntegrator2());
    vIntegrator_.reset(new MacCormackIntegrator2());
    integrator_.reset(new MacCormackIntegrator2());
    addScalarField(); // 0 density
    addScalarField(); // 1 temperature
  }
  ~GridSmokeSolver2() {
    __freeScene<<<1, 1>>>(scene_.list);
    using namespace hermes::cuda;
    CUDA_CHECK(cudaFree(scene_.list));
    CUDA_CHECK(cudaFree(scene_.colliders));
  }
  void setUIntegrator(Integrator2 *integrator) {
    uIntegrator_.reset(integrator);
  }
  void setVIntegrator(Integrator2 *integrator) {
    vIntegrator_.reset(integrator);
  }
  void setIntegrator(Integrator2 *integrator) { integrator_.reset(integrator); }
  void init() {
    using namespace hermes::cuda;
    CUDA_CHECK(cudaMalloc(&scene_.list, 6 * sizeof(Collider2<float> *)));
    CUDA_CHECK(cudaMalloc(&scene_.colliders, sizeof(Collider2<float> *)));
    hermes::cuda::fill2(scene_.target_temperature.data(), 273.f);
    hermes::cuda::fill2(scene_.smoke_source.data(), (unsigned char)0);
    for (size_t i = 0; i < 2; i++) {
      hermes::cuda::fill2(velocity_[i].u().data(), 0.f);
      hermes::cuda::fill2(velocity_[i].v().data(), 0.f);
      hermes::cuda::fill2(scalarFields_[i][0].data(), 0.f);
      hermes::cuda::fill2(scalarFields_[i][1].data(), 273.f);
    }
    hermes::cuda::fill2(solidScalarFields_[0].data(), 0.f);
    hermes::cuda::fill2(solidScalarFields_[1].data(), 273.f);
    hermes::cuda::fill2(vorticityField_.u().data(), 0.f);
    hermes::cuda::fill2(vorticityField_.v().data(), 0.f);
  }
  ///
  /// \param res
  void setResolution(const ponos::size2 &res) {
    resolution_ = hermes::cuda::vec2u(res.width, res.height);
    for (size_t i = 0; i < 2; i++) {
      velocity_[i].resize(resolution_);
      for (auto &f : scalarFields_[i])
        f.resize(resolution_);
    }
    for (auto &f : solidScalarFields_)
      f.resize(resolution_);
    vorticityField_.resize(resolution_);
    pressure_.resize(resolution_);
    divergence_.resize(resolution_);
    solid_.resize(resolution_);
    solidVelocity_.resize(resolution_);
    forceField_.resize(resolution_);
    integrator_->set(scalarFields_[0][0].info());
    uIntegrator_->set(velocity_[0].u().info());
    vIntegrator_->set(velocity_[0].v().info());
    pressureMatrix_.resize(resolution_);
    scene_.resize(resolution_);
  }
  /// Sets cell size
  /// \param _dx scale
  void setSpacing(const ponos::vec2f &s) {
    spacing_ = hermes::cuda::vec2f(s.x, s.y);
    for (size_t i = 0; i < 2; i++) {
      velocity_[i].setSpacing(spacing_);
      for (auto &f : scalarFields_[i])
        f.setSpacing(spacing_);
    }
    for (auto &f : solidScalarFields_)
      f.setSpacing(spacing_);
    vorticityField_.setSpacing(spacing_);
    pressure_.setSpacing(spacing_);
    divergence_.setSpacing(spacing_);
    solid_.setSpacing(spacing_);
    solidVelocity_.setSpacing(spacing_);
    forceField_.setSpacing(spacing_);
    integrator_->set(scalarFields_[0][0].info());
    uIntegrator_->set(velocity_[0].u().info());
    vIntegrator_->set(velocity_[0].v().info());
    scene_.setSpacing(spacing_);
  }
  /// Sets lower left corner position
  /// \param o offset
  void setOrigin(const ponos::point2f &o) {
    hermes::cuda::point2f p(o.x, o.y);
    for (size_t i = 0; i < 2; i++) {
      velocity_[i].setOrigin(p);
      for (auto &f : scalarFields_[i])
        f.setOrigin(p);
    }
    for (auto &f : solidScalarFields_)
      f.setOrigin(p);
    vorticityField_.setOrigin(p);
    pressure_.setOrigin(p);
    divergence_.setOrigin(p);
    solid_.setOrigin(p);
    solidVelocity_.setOrigin(p);
    forceField_.setOrigin(p);
    integrator_->set(scalarFields_[0][0].info());
    uIntegrator_->set(velocity_[0].u().info());
    vIntegrator_->set(velocity_[0].v().info());
  }
  size_t addScalarField() {
    scalarFields_[src].emplace_back();
    scalarFields_[dst].emplace_back();
    solidScalarFields_.emplace_back();
    solidScalarFields_.emplace_back();
    return scalarFields_[src].size() - 1;
  }
  /// Advances one simulation step
  /// \param dt time step
  void step(float dt) {
    // rasterColliders();
    // uIntegrator_->advect(velocity_[src], solid_, solidVelocity_.u(),
    //                      velocity_[src].u(), velocity_[dst].u(), dt);
    // vIntegrator_->advect(velocity_[src], solid_, solidVelocity_.v(),
    //                      velocity_[src].v(), velocity_[dst].v(), dt);
    velocity_[dst].copy(velocity_[src]);
    for (size_t i = 0; i < scalarFields_[src].size(); i++)
      integrator_->advect(velocity_[dst], solid_, solidScalarFields_[i],
                          scalarFields_[src][i], scalarFields_[dst][i], dt);
    src = src ? 0 : 1;
    dst = dst ? 0 : 1;
    hermes::cuda::fill2(forceField_.u().data(), 0.f);
    hermes::cuda::fill2(forceField_.v().data(), 0.f);
    addBuoyancyForce(forceField_, solid_, scalarFields_[src][0],
                     scalarFields_[src][1], 273, 1.0f, 0.0f);
    // addVorticityConfinementForce(forceField_, velocity_[src],
    // solid_,vorticityField_, 2.f, dt);
    // applyForceField(velocity_[src], solid_, forceField_, dt);
    // injectSmoke(scalarFields_[src][0], scene_.smoke_source, dt);
    // injectTemperature(scalarFields_[src][1], scene_.target_temperature, dt);
    computeDivergence(velocity_[src], solid_, solidVelocity_, divergence_);
    solvePressureSystem(pressureMatrix_, divergence_, pressure_, solid_, dt);
    projectionStep_t(pressure_, solid_, velocity_[src], dt);
  }
  /// Raster collider bodies and velocities into grid simulations
  void rasterColliders() {
    __setupScene<<<1, 1>>>(scene_.list, scene_.colliders, resolution_.x);
    hermes::ThreadArrayDistributionInfo td(resolution_);
    __rasterColliders<<<td.gridSize, td.blockSize>>>(
        scene_.colliders, solid_.accessor(), solidVelocity_.u().accessor(),
        solidVelocity_.v().accessor());
  }
  Scene2<float> &scene() { return scene_; }
  hermes::cuda::RegularGrid2Df &scalarField(size_t i) {
    return scalarFields_[src][i];
  }
  hermes::cuda::StaggeredGrid2D &velocity() { return velocity_[src]; }
  hermes::cuda::RegularGrid2Duc &solid() { return solid_; }
  hermes::cuda::RegularGrid2Df &divergence() { return divergence_; }

private:
  Scene2<float> scene_;
  std::shared_ptr<Integrator2> vIntegrator_;
  std::shared_ptr<Integrator2> uIntegrator_;
  std::shared_ptr<Integrator2> integrator_;
  poseidon::cuda::FDMatrix2D pressureMatrix_;
  hermes::cuda::StaggeredGrid2D velocity_[2];
  hermes::cuda::StaggeredGrid2D solidVelocity_;
  hermes::cuda::VectorGrid2D forceField_, vorticityField_;
  hermes::cuda::RegularGrid2Df pressure_;
  hermes::cuda::RegularGrid2Df divergence_;
  hermes::cuda::RegularGrid2Duc solid_;
  std::vector<hermes::cuda::RegularGrid2Df> scalarFields_[2];
  std::vector<hermes::cuda::RegularGrid2Df> solidScalarFields_;
  hermes::cuda::vec2u resolution_;
  hermes::cuda::vec2f spacing_;
  size_t src = 0;
  size_t dst = 1;
};
/// Eulerian grid based solver for smoke simulations. Stores its data in fast
/// device texture memory.
class GridSmokeSolver3 {
public:
  GridSmokeSolver3() {
    uIntegrator_.reset(new MacCormackIntegrator3());
    vIntegrator_.reset(new MacCormackIntegrator3());
    wIntegrator_.reset(new MacCormackIntegrator3());
    integrator_.reset(new MacCormackIntegrator3());
    addScalarField(); // 0 density
    addScalarField(); // 1 temperature
  }
  ~GridSmokeSolver3() {
    __freeScene<<<1, 1>>>(scene_.list);
    using namespace hermes::cuda;
    CUDA_CHECK(cudaFree(scene_.list));
    CUDA_CHECK(cudaFree(scene_.colliders));
  }
  void setUIntegrator(Integrator3 *integrator) {
    uIntegrator_.reset(integrator);
  }
  void setVIntegrator(Integrator3 *integrator) {
    vIntegrator_.reset(integrator);
  }
  void setWIntegrator(Integrator3 *integrator) {
    wIntegrator_.reset(integrator);
  }
  void setIntegrator(Integrator3 *_integrator) {
    integrator_.reset(_integrator);
  }
  void init() {
    using namespace hermes::cuda;
    CUDA_CHECK(cudaMalloc(&scene_.list, 6 * sizeof(Collider3<float> *)));
    CUDA_CHECK(cudaMalloc(&scene_.colliders, sizeof(Collider3<float> *)));
    hermes::cuda::fill3(scene_.target_temperature.data().accessor(), 273.f);
    hermes::cuda::fill3(scene_.smoke_source.data().accessor(),
                        (unsigned char)0);
    for (size_t i = 0; i < 2; i++) {
      hermes::cuda::fill3(velocity_[i].u().data().accessor(), 0.f);
      hermes::cuda::fill3(velocity_[i].v().data().accessor(), 0.f);
      hermes::cuda::fill3(velocity_[i].w().data().accessor(), 0.f);
      hermes::cuda::fill3(scalarFields_[i][0].data().accessor(), 0.f);
      hermes::cuda::fill3(scalarFields_[i][1].data().accessor(), 273.f);
    }
    hermes::cuda::fill3(vorticityField_.u().data().accessor(), 0.f);
    hermes::cuda::fill3(vorticityField_.v().data().accessor(), 0.f);
    hermes::cuda::fill3(vorticityField_.w().data().accessor(), 0.f);
  }
  ///
  /// \param res
  void setResolution(const ponos::size3 &res) {
    resolution_ = hermes::cuda::vec3u(res.width, res.height, res.depth);
    for (size_t i = 0; i < 2; i++) {
      velocity_[i].resize(resolution_);
      for (auto &f : scalarFields_[i])
        f.resize(resolution_);
    }
    vorticityField_.resize(resolution_);
    pressure_.resize(resolution_);
    divergence_.resize(resolution_);
    solid_.resize(resolution_);
    solidVelocity_.resize(resolution_);
    forceField_.resize(resolution_);
    integrator_->set(scalarFields_[0][0].info());
    uIntegrator_->set(velocity_[0].u().info());
    vIntegrator_->set(velocity_[0].v().info());
    wIntegrator_->set(velocity_[0].w().info());
    pressureMatrix_.resize(resolution_);
    scene_.resize(resolution_);
  }
  /// Sets cell size
  /// \param _dx scale
  void setSpacing(const ponos::vec3f &s) {
    spacing_ = hermes::cuda::vec3f(s.x, s.y, s.z);
    for (size_t i = 0; i < 2; i++) {
      velocity_[i].setSpacing(spacing_);
      for (auto &f : scalarFields_[i])
        f.setSpacing(spacing_);
    }
    vorticityField_.setSpacing(spacing_);
    pressure_.setSpacing(spacing_);
    divergence_.setSpacing(spacing_);
    solid_.setSpacing(spacing_);
    solidVelocity_.setSpacing(spacing_);
    forceField_.setSpacing(spacing_);
    integrator_->set(scalarFields_[0][0].info());
    uIntegrator_->set(velocity_[0].u().info());
    vIntegrator_->set(velocity_[0].v().info());
    wIntegrator_->set(velocity_[0].w().info());
    scene_.setSpacing(spacing_);
  }
  /// Sets lower left corner position
  /// \param o offset
  void setOrigin(const ponos::point3f &o) {
    hermes::cuda::point3f p(o.x, o.y, o.z);
    for (size_t i = 0; i < 2; i++) {
      velocity_[i].setOrigin(p);
      for (auto &f : scalarFields_[i])
        f.setOrigin(p);
    }
    vorticityField_.setOrigin(p);
    pressure_.setOrigin(p);
    divergence_.setOrigin(p);
    solid_.setOrigin(p);
    solidVelocity_.setOrigin(p);
    forceField_.setOrigin(p);
    integrator_->set(scalarFields_[0][0].info());
    uIntegrator_->set(velocity_[0].u().info());
    vIntegrator_->set(velocity_[0].v().info());
    wIntegrator_->set(velocity_[0].w().info());
  }
  size_t addScalarField() {
    scalarFields_[src].emplace_back();
    scalarFields_[dst].emplace_back();
    return scalarFields_[src].size() - 1;
  }
  /// Advances one simulation step
  /// \param dt time step
  void step(float dt) {
    // rasterColliders();
    // uIntegrator_->advect(velocity_[src], solid_, velocity_[src].u(),
    //                      velocity_[dst].u(), dt);
    // vIntegrator_->advect(velocity_[src], solid_, velocity_[src].v(),
    //                      velocity_[dst].v(), dt);
    // wIntegrator_->advect(velocity_[src], solid_, velocity_[src].w(),
    //                      velocity_[dst].w(), dt);
    velocity_[dst].copy(velocity_[src]);
    for (size_t i = 0; i < scalarFields_[src].size(); i++)
      integrator_->advect(velocity_[dst], solid_, scalarFields_[src][i],
                          scalarFields_[dst][i], dt);
    src = src ? 0 : 1;
    dst = dst ? 0 : 1;
    return;
    hermes::cuda::fill3(forceField_.u().data().accessor(), 0.f);
    hermes::cuda::fill3(forceField_.v().data().accessor(), 0.f);
    hermes::cuda::fill3(forceField_.w().data().accessor(), 0.f);
    addBuoyancyForce(forceField_, solid_, scalarFields_[src][0],
                     scalarFields_[src][1], 273, 1.0f, 0.04f);
    addVorticityConfinementForce(forceField_, velocity_[src], solid_,
                                 vorticityField_, 2.f, dt);
    applyForceField(velocity_[src], solid_, forceField_, dt);
    injectSmoke(scalarFields_[src][0], scene_.smoke_source, dt);
    injectTemperature(scalarFields_[src][1], scene_.target_temperature, dt);
    computeDivergence(velocity_[src], solid_, divergence_);
    solvePressureSystem(pressureMatrix_, divergence_, pressure_, solid_, dt);
    projectionStep(pressure_, solid_, velocity_[src], dt);
  }
  /// Raster collider bodies and velocities into grid simulations
  void rasterColliders() {
    __setupScene<<<1, 1>>>(scene_.list, scene_.colliders, resolution_.x);
    hermes::ThreadArrayDistributionInfo td(resolution_);
    __rasterColliders<<<td.gridSize, td.blockSize>>>(
        scene_.colliders, solid_.accessor(), solidVelocity_.u().accessor(),
        solidVelocity_.v().accessor(), solidVelocity_.w().accessor());
  }
  Scene3<float> &scene() { return scene_; }
  hermes::cuda::RegularGrid3Df &scalarField(size_t i) {
    return scalarFields_[src][i];
  }
  hermes::cuda::StaggeredGrid3D &velocity() { return velocity_[src]; }
  hermes::cuda::RegularGrid3Duc &solid() { return solid_; }
  hermes::cuda::RegularGrid3Df &divergence() { return divergence_; }

private:
  Scene3<float> scene_;
  std::shared_ptr<Integrator3> wIntegrator_;
  std::shared_ptr<Integrator3> vIntegrator_;
  std::shared_ptr<Integrator3> uIntegrator_;
  std::shared_ptr<Integrator3> integrator_;
  poseidon::cuda::FDMatrix3D pressureMatrix_;
  hermes::cuda::StaggeredGrid3D velocity_[2];
  hermes::cuda::StaggeredGrid3D solidVelocity_;
  hermes::cuda::VectorGrid3D forceField_, vorticityField_;
  hermes::cuda::RegularGrid3Df pressure_;
  hermes::cuda::RegularGrid3Df divergence_;
  hermes::cuda::RegularGrid3Duc solid_;
  std::vector<hermes::cuda::RegularGrid3Df> scalarFields_[2];
  hermes::cuda::vec3u resolution_;
  hermes::cuda::vec3f spacing_;
  size_t src = 0;
  size_t dst = 1;
};

} // namespace cuda

} // namespace poseidon

#endif // POSEIDON_SOLVERS_CUDA_SMOKE_SOLVER_H