template <typename ParticleType>
FLIP2DScene<ParticleType>::FLIP2DScene(const char *filename,
                                       ZParticleGrid2D<ParticleType> *_pg)
    : pg(_pg) {
  load(filename);
  rng.setBase(3);
}

template <typename ParticleType> FLIP2DScene<ParticleType>::~FLIP2DScene() {}

template <typename ParticleType>
void FLIP2DScene<ParticleType>::load(const char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp)
    return;
  char line[1000];
  while (fgets(line, 1000, fp) != nullptr) {
    char command = line[0];
    char type;
    int n;
    fscanf(fp, "%c %d", &type, &n);
    ponos::RawMesh *m = new ponos::RawMesh();
    for (int i = 0; i < n; i++) {
      ponos::Point2 p;
      fscanf(fp, " %f %f ", &p.x, &p.y);
      if (type == 'i')
        p = pg->toWorld(p);
      m->vertices.emplace_back(p.x);
      m->vertices.emplace_back(p.y);
    }
    m->vertexDescriptor.count = n;
    m->meshDescriptor.elementSize = 2;
    m->meshDescriptor.count = n;
    m->indices.resize(n * 2);
    for (int i = 0; i < n; i++) {
      m->indices[i * 2].vertexIndex = i;
      m->indices[i * 2 + 1].vertexIndex = (i + 1) % n;
    }
    m->vertexDescriptor.elementSize = 2;
    m->computeBBox();
    switch (command) {
    case 'l':
      liquidsGeometry.emplace_back(new ponos::Mesh2D(m, ponos::Transform2D()));
      break;
    case 's':
      solidsGeometry.emplace_back(new ponos::Mesh2D(m, ponos::Transform2D()));
      break;
    case 'p':
      staticSolidsGeometry.emplace_back(
          new ponos::Mesh2D(m, ponos::Transform2D()));
      break;
    }
  }
  fclose(fp);
}

template <typename ParticleType>
void FLIP2DScene<ParticleType>::addForce(const ponos::vec2 &f) {
  forces.emplace_back(f);
}

template <typename ParticleType>
void FLIP2DScene<ParticleType>::generateParticles(int frame) {
  // generate liquid particles
  for (size_t l = 0; l < liquidsGeometry.size(); l++) {
    ponos::BBox2D bbox = liquidsGeometry[l]->getBBox();
    ponos::ivec2 lmin = ponos::floor(ponos::vec2(pg->toGrid(bbox.pMin)));
    ponos::ivec2 lmax = ponos::ceil(ponos::vec2(pg->toGrid(bbox.pMax)));
    lmin = ponos::max(lmin, ponos::ivec2());
    lmax = ponos::min(lmax, pg->dimensions + ponos::ivec2(1.0f));
    // ponos::parallel_for(lmin[0], lmax[0] + 1, [=](size_t start, size_t end) {
    size_t start = 0;
    size_t end = lmax[0] + 1;
    ponos::ivec2 m(start, lmin[1]);
    ponos::ivec2 M(end, lmax[1] + 1);
    ponos::ivec2 ijk;
    FOR_INDICES2D(m, M, ijk) { fillCell(frame, liquidsGeometry[l], ijk.xy()); }
    // });
  }
  // generate static solid particles
  for (size_t l = 0; l < staticSolidsGeometry.size(); l++) {
    ponos::BBox2D bbox = staticSolidsGeometry[l]->getBBox();
    ponos::ivec2 lmin = ponos::floor(ponos::vec2(pg->toGrid(bbox.pMin)));
    ponos::ivec2 lmax = ponos::ceil(ponos::vec2(pg->toGrid(bbox.pMax)));
    lmin = ponos::max(lmin, ponos::ivec2());
    lmax = ponos::min(lmax, pg->dimensions + ponos::ivec2(1.0f));
    // ponos::parallel_for(lmin[0], lmax[0] + 1, [=](size_t start, size_t end) {
    size_t start = 0;
    size_t end = lmax[0] + 1;
    ponos::ivec2 m(start, lmin[1]);
    ponos::ivec2 M(end, lmax[1] + 1);
    ponos::ivec2 ijk;
    FOR_INDICES2D(m, M, ijk) {
      ponos::Point2 wp =
          pg->toWorld(ponos::Point2(ijk[0], ijk[1]) + ponos::vec2(0.5f));
      if (staticSolidsGeometry[l]->intersect(wp)) {
        ParticleType *p = new ParticleType;
        p->position = wp;
        p->velocity = ponos::vec2();
        p->density = 10.0f;
        p->type = ParticleTypes::SOLID;
        p->mass = 1.0f;
        p->invalid = false;
        staticSolidParticles.emplace_back(p);
      }
    }
    //});
  }
  for (auto p : liquidParticles)
    pg->addParticle(p);
  for (auto p : solidParticles)
    pg->addParticle(p);
  for (auto p : staticSolidParticles)
    pg->addParticle(p);
  pg->update();
}

template <typename ParticleType>
void FLIP2DScene<ParticleType>::fillCell(int frame,
                                         ponos::Mesh2D *liquidGeometry,
                                         const ponos::ivec2 &ij) {
  float directions[4][2] = {{.25, .25}, {.25, .75}, {.75, .25}, {.75, .75}};
  for (int i = 0; i < 4; i++) {
    ponos::Point2 wp = pg->toWorld(
        ponos::Point2(ij[0], ij[1]) +
        ponos::vec2(directions[i][0], directions[i][1]) +
        ponos::vec2(rng.randomFloat() - 0.5f, rng.randomFloat() - 0.5f) * .5f);
    if (liquidGeometry->intersect(wp)) {
      size_t solidGeomID = 0;
      if (!isPointInsideSolid(wp, frame, &solidGeomID)) {
        ParticleType *p = new ParticleType;
        p->position = wp;
        p->velocity = ponos::vec2(0, 0);
        p->density = 10.0f;
        p->type = ParticleTypes::FLUID;
        p->mass = 1.0f;
        p->invalid = false;
        liquidParticles.emplace_back(p);
      }
    }
  }
}

template <typename ParticleType>
const std::vector<ponos::Mesh2D *> &
FLIP2DScene<ParticleType>::getLiquidsGeometry() const {
  return liquidsGeometry;
}

template <typename ParticleType>
const std::vector<ponos::Mesh2D *> &
FLIP2DScene<ParticleType>::getStaticSolidsGeometry() const {
  return staticSolidsGeometry;
}

template <typename ParticleType>
const std::vector<ponos::Mesh2D *> &
FLIP2DScene<ParticleType>::getSolidsGeometry() const {
  return solidsGeometry;
}

template <typename ParticleType>
const std::vector<ParticleType *> &
FLIP2DScene<ParticleType>::getLiquidParticles() const {
  return liquidParticles;
}

template <typename ParticleType>
const std::vector<ponos::vec2> &
FLIP2DScene<ParticleType>::getForces(int f) const {
  return forces;
}

template <typename ParticleType>
void FLIP2DScene<ParticleType>::buildStaticSolidLevelSet() {
  for (size_t i = 0; i < staticSolidsGeometry.size(); i++) {
    if (i == 0)
      staticSolidLevelSet.reset(
          new ponos::LevelSet2D(staticSolidsGeometry[i]->getMesh(),
                                staticSolidsGeometry[i]->getTransform()));
    else
      staticSolidLevelSet->merge(
          new ponos::LevelSet2D(staticSolidsGeometry[i]->getMesh(),
                                staticSolidsGeometry[i]->getTransform()));
  }
}

template <typename ParticleType>
void FLIP2DScene<ParticleType>::buildSolidLevelSet() {
  for (size_t i = 0; i < solidsGeometry.size(); i++) {
    if (i == 0)
      staticSolidLevelSet.reset(new ponos::LevelSet2D(
          solidsGeometry[i]->getMesh(), solidsGeometry[i]->getTransform()));
    else
      staticSolidLevelSet->merge(new ponos::LevelSet2D(
          solidsGeometry[i]->getMesh(), solidsGeometry[i]->getTransform()));
  }
  if (solidsGeometry.size())
    solidLevelSet->copy(staticSolidLevelSet.get());
  else
    solidLevelSet->merge(staticSolidLevelSet.get());
}

template <typename ParticleType>
bool FLIP2DScene<ParticleType>::isPointInsideSolid(const ponos::Point2 &p,
                                                   int f, size_t *id) const {
  for (size_t i = 0; i < solidsGeometry.size(); i++) {
    if (solidsGeometry[i]->intersect(p)) {
      *id = i;
      return true;
    }
  }
  return false;
}

template <typename ParticleType>
FLIP2D<ParticleType>::FLIP2D(const char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp)
    return;
  char parameter[1000];
  while (fscanf(fp, " %s ", parameter) != EOF) {
    if (!strcmp(parameter, "dimensions")) {
      fscanf(fp, " %d %d ", &dimensions[0], &dimensions[1]);
    } else if (!strcmp(parameter, "density")) {
      fscanf(fp, " %f ", &density);
    } else if (!strcmp(parameter, "pic_flip_ratio")) {
      fscanf(fp, " %f ", &pic_flip_ratio);
    } else if (!strcmp(parameter, "subcell")) {
      fscanf(fp, " %d ", &subcell);
    } else if (!strcmp(parameter, "dt")) {
      fscanf(fp, " %f ", &dt);
    } else if (!strcmp(parameter, "dx")) {
      fscanf(fp, " %f ", &dx);
    } else if (!strcmp(parameter, "curStep")) {
      fscanf(fp, " %d ", &curStep);
    }
  }
  fclose(fp);

  setup();
}

template <typename ParticleType>
FLIP2D<ParticleType>::FLIP2D(FLIP2DScene<ParticleType> *s) {
  scene.reset(s);
}

template <typename ParticleType>
void FLIP2D<ParticleType>::loadScene(const char *filename) {
  scene.reset(new FLIP2DScene<ParticleType>(filename, particleGrid.get()));
}

template <typename ParticleType> void FLIP2D<ParticleType>::setup() {
  COPY_GRID = 0;
  CUR_GRID = 1;
  particleGrid.reset(new ZParticleGrid2D<ParticleType>(
      dimensions[0], dimensions[1],
      ponos::BBox2D(ponos::Point2(),
                    ponos::Point2(dimensions[0] * dx, dimensions[1] * dx))));
  for (int i = 0; i < 2; i++)
    grid[i] =
        new MacGrid2D<ponos::CRegularGrid2D>(dimensions[0], dimensions[1], dx);
  distances.setDimensions(dimensions[0], dimensions[1]);
  usolid.setDimensions(dimensions[0] + 1, dimensions[1]);
  vsolid.setDimensions(dimensions[0], dimensions[1] + 1);
  usolid.setAll(0.f);
  vsolid.setAll(0.f);
  solver.set(dimensions[0] * dimensions[1]);
}

template <typename ParticleType> bool FLIP2D<ParticleType>::init() {
  ZParticleGrid2D<ParticleType> pg(
      10, 10, ponos::BBox2D(ponos::Point2(), ponos::Point2(10 * dx, 10 * dx)));
  ponos::ivec2 ij;
  FOR_INDICES0_2D(ponos::ivec2(10, 10), ij) {
    ponos::Point2 cp((ij[0] + 0.5f) * dx, (ij[1] + 0.5f) * dx);
    pg.add(cp + ponos::vec2(-0.25f, -0.25f) * dx);
    pg.add(cp + ponos::vec2(0.25f, -0.25f) * dx);
    pg.add(cp + ponos::vec2(0.25f, 0.25f) * dx);
    pg.add(cp + ponos::vec2(0.25f, 0.25f) * dx);
  }
  maxDensity = 1.f;
  scene->generateParticles(0);
  markCells();
  return true;
}

template <typename ParticleType> void FLIP2D<ParticleType>::step() {
  // scene->generateParticles(curStep);
  // scene->buildSolidLevelSet(curStep);
  // adjustParticlesStuckInSolids();

  advectParticles();
  // resampleParticles();
  markCells();
  // copy velocities of particles
  particleGrid->iterateAll([](ParticleType *p) {
    p->p_copy = p->position;
    p->v_copy = p->velocity;
  });
  // computeDensity();
  sendVelocitiesToGrid();
  copyGridVelocities();
  applyExternalForces();
  computeDistanceToFluid();
  extrapolateVelocity();
  enforceBoundaries();
  grid[CUR_GRID]->computeNegativeDivergence();
  project();
  extrapolateVelocity();
  enforceBoundaries();
  subtractGrid();
  solvePICFLIP();

  curStep++;
}

template <typename ParticleType> float FLIP2D<ParticleType>::CFL() {
  float maxv2 = 0.f;
  const std::vector<ponos::vec2> &forces = scene->getForces(curStep);
  for (auto f : forces)
    maxv2 = std::max(maxv2, dx * std::max(f[0], f[1]));
  maxv2 = std::max(maxv2, SQR(grid[CUR_GRID]->v_u->infNorm()) +
                              SQR(grid[CUR_GRID]->v_v->infNorm()));
  if (maxv2 < 1e-16)
    maxv2 = 1e-16;
  return dx / sqrt(maxv2);
}

template <typename ParticleType> void FLIP2D<ParticleType>::markCells() {
  ponos::ivec2 ij;
  FOR_INDICES0_2D(dimensions, ij) {
    (*grid[CUR_GRID]->cellType)(ij) = CellType::AIR;
    typename ZParticleGrid2D<ParticleType>::particle_iterator it =
        particleGrid->getCell(ij);
    while (it.next()) {
      if ((*it)->type == ParticleTypes::SOLID) {
        (*grid[CUR_GRID]->cellType)(ij) = CellType::SOLID;
        break;
      }
      (*grid[CUR_GRID]->cellType)(ij) = CellType::FLUID;
      ++it;
    }
  }
}

template <typename ParticleType> void FLIP2D<ParticleType>::computeDensity() {
  particleGrid->iterateAll([this](ParticleType *p) {
    if (p->type == ParticleTypes::SOLID)
      p->density = 1.f;
    else {
      float radius = 1.5f * this->dx;
      typename ZParticleGrid2D<
          ParticleType>::template WeightedAverageAccumulator<float>
          accumulator(radius, ParticleAttribute::DENSITY);
      this->particleGrid->tree_->searchAndApply(p->position, radius,
                                                accumulator);
      p->density = accumulator.result();
    }
  });
}

template <typename ParticleType>
void FLIP2D<ParticleType>::applyExternalForces() {
  const std::vector<ponos::vec2> &forces = scene->getForces(curStep);
  ponos::ivec2 ij;
  FOR_INDICES0_2D(grid[CUR_GRID]->v_v->getDimensions(), ij) {
    for (auto f : forces)
      (*grid[CUR_GRID]->v_v)(ij) += f[1] * dt;
  }
  /*
const std::vector<ponos::vec2> &forces = scene->getForces(curStep);
particleGrid->iterateAll([this, &forces](ParticleType *p) {
if (p->type != ParticleTypes::FLUID)
return;
for (auto f : forces) {
p->velocity += f * this->dt;
}
});*/
}

template <typename ParticleType>
void FLIP2D<ParticleType>::sendVelocitiesToGrid() {
  float radius = 1.5f * dx;
  ponos::ivec2 ij;
  // splat U velocaities
  FOR_INDICES0_2D(grid[CUR_GRID]->v_u->getDimensions(), ij) {
    ponos::Point2 wp = grid[CUR_GRID]->v_u->dataWorldPosition(ij);
    (*grid[CUR_GRID]->v_u)(ij[0], ij[1]) =
        particleGrid->gather(ParticleAttribute::VELOCITY_X, wp, radius);
  }
  // splat V velocities
  FOR_INDICES0_2D(grid[CUR_GRID]->v_v->getDimensions(), ij) {
    ponos::Point2 wp = grid[CUR_GRID]->v_v->dataWorldPosition(ij);
    (*grid[CUR_GRID]->v_v)(ij[0], ij[1]) =
        particleGrid->gather(ParticleAttribute::VELOCITY_Y, wp, radius);
  }
}

template <typename ParticleType>
void FLIP2D<ParticleType>::enforceBoundaries() {
  ponos::ivec2 ij;
  // X directions
  FOR_INDICES0_2D(grid[CUR_GRID]->v_u->getDimensions(), ij) {
    int a = ((*grid[CUR_GRID]->cellType)(ij + ponos::ivec2(-1, 0)) ==
             CellType::SOLID)
                ? -1
                : 1;
    int b = ((*grid[CUR_GRID]->cellType)(ij + ponos::ivec2(0, 0)) ==
             CellType::SOLID)
                ? -1
                : 1;
    if (a * b < 0)
      (*grid[CUR_GRID]->v_u)(ij) = 0.f;
  }
  // Y directions
  FOR_INDICES0_2D(grid[CUR_GRID]->v_v->getDimensions(), ij) {
    int a = ((*grid[CUR_GRID]->cellType)(ij + ponos::ivec2(0, -1)) ==
             CellType::SOLID)
                ? -1
                : 1;
    int b = ((*grid[CUR_GRID]->cellType)(ij + ponos::ivec2(0, 0)) ==
             CellType::SOLID)
                ? -1
                : 1;
    if (a * b < 0)
      (*grid[CUR_GRID]->v_v)(ij) = 0.f;
  }
}

template <typename ParticleType>
int FLIP2D<ParticleType>::matrixIndex(int i, int j) {
  return dimensions[0] * i + j;
}

template <typename ParticleType> void FLIP2D<ParticleType>::project() {
  solver.reset();
  ponos::ivec2 ij;
  ponos::CRegularGrid2D<float> &v = *grid[CUR_GRID]->v_v;
  ponos::CRegularGrid2D<float> &u = *grid[CUR_GRID]->v_u;
  ponos::CRegularGrid2D<float> &d = *grid[CUR_GRID]->D;
  ponos::RegularGrid2D<CellType> &cell = *grid[CUR_GRID]->cellType;
  // build rhs and account for solid velocities
  float s = 1.f / dx;
  FOR_INDICES0_2D(dimensions, ij) {
    int i = ij[0], j = ij[1], ind = matrixIndex(i, j);
    if (cell(ij) == CellType::FLUID) {
      solver.setB(matrixIndex(i, j), d(ij));
      if (cell(i - 1, j) == CellType::SOLID)
        solver.incrementB(ind, -(s * (u(i, j) - usolid(i, j))));
      if (cell(i + 1, j) == CellType::SOLID)
        solver.incrementB(ind, (s * (u(i + 1, j) - usolid(i + 1, j))));
      if (cell(i, j - 1) == CellType::SOLID)
        solver.incrementB(ind, -(s * (v(i, j) - vsolid(i, j))));
      if (cell(i, j + 1) == CellType::SOLID)
        solver.incrementB(ind, (s * (v(i, j + 1) - vsolid(i, j + 1))));
    }
  }
  // for(int i = 0; i < dimensions[0] * dimensions[1]; i++)
  //	std::cout << solver.B.coeffRef(i) << " ";
  //	std::cout << std::endl;
  // set up the matrix
  s = dt / (density * dx * dx);
  FOR_INDICES0_2D(dimensions, ij) {
    int i = ij[0], j = ij[1];
    if (cell(ij) == CellType::FLUID) {
      int solidCount = 0;
      if (cell(i + 1, j) == CellType::FLUID)
        solver.setA(matrixIndex(i, j), matrixIndex(i + 1, j), -s);
      else if (cell(i + 1, j) == CellType::SOLID)
        solidCount++;
      if (cell(i - 1, j) == CellType::FLUID)
        solver.setA(matrixIndex(i, j), matrixIndex(i - 1, j), -s);
      else if (cell(i - 1, j) == CellType::SOLID)
        solidCount++;
      if (cell(i, j + 1) == CellType::FLUID)
        solver.setA(matrixIndex(i, j), matrixIndex(i, j + 1), -s);
      else if (cell(i, j + 1) == CellType::SOLID)
        solidCount++;
      if (cell(i, j - 1) == CellType::FLUID)
        solver.setA(matrixIndex(i, j), matrixIndex(i, j - 1), -s);
      else if (cell(i, j - 1) == CellType::SOLID)
        solidCount++;
      solver.setA(matrixIndex(i, j), matrixIndex(i, j), s * (4 - solidCount));
    }
  }
  solver.solve();
  // for(int i = 0; i < dimensions[0] * dimensions[1]; i++) {
  //	for(int j = 0; j < dimensions[0] * dimensions[1]; j++) {
  //		std::cout << solver.A.coeffRef(i, j) << " ";
  //	}
  //	std::cout << ";\n";
  //}
  // for(int i = 0; i < dimensions[0] * dimensions[1]; i++)
  //	std::cout << solver.X.coeffRef(i) << "\n";

  // pressure update
  s = dt / (density * dx);
  ponos::ivec2 IJ = u.getDimensions() - ponos::ivec2(1, 0);
  FOR_INDICES2D(ponos::ivec2(1, 0), IJ, ij) {
    int i = ij[0], j = ij[1];
    // if (cell(ij) == CellType::FLUID) {
    u(i, j) -= s * (solver.getX(matrixIndex(i, j)) -
                    solver.getX(matrixIndex(i - 1, j)));
    //}
  }
  IJ = v.getDimensions() - ponos::ivec2(0, 1);
  FOR_INDICES2D(ponos::ivec2(0, 1), IJ, ij) {
    int i = ij[0], j = ij[1];
    // if (cell(ij) == CellType::FLUID) {
    v(i, j) -= s * (solver.getX(matrixIndex(i, j)) -
                    solver.getX(matrixIndex(i, j - 1)));
    //}
  }
  FOR_INDICES0_2D(dimensions, ij) {
    int i = ij[0], j = ij[1];
    if (cell(ij) == CellType::SOLID) {
      if (cell(i + 1, j) == CellType::FLUID)
        u(i + 1, j) = 0;
      if (cell(i - 1, j) == CellType::FLUID)
        u(i, j) = 0;
      if (cell(i, j + 1) == CellType::FLUID)
        v(i, j + 1) = 0;
      if (cell(i, j - 1) == CellType::FLUID)
        v(i, j) = 0;
      //  u(i, j) = usolid(i, j);
      //  u(i + 1, j) = usolid(i + 1, j);
      //  v(i, j) = vsolid(i, j);
      //  v(i, j + 1) = vsolid(i, j + 1);
    }
  }
}

template <typename ParticleType>
void FLIP2D<ParticleType>::computeDistanceToFluid() {
  ponos::RegularGrid2D<CellType> *cell = grid[CUR_GRID]->cellType.get();
  distances.setAll(dimensions[0] + 2 * dimensions[1]);
  ponos::ivec2 ij;
  FOR_INDICES0_2D(grid[CUR_GRID]->cellType->getDimensions(), ij) {
    if ((*grid[CUR_GRID]->cellType)(ij) == CellType::FLUID) {
      distances(ij) = -0.5f;
    }
  }
  for (int i = 0; i < 2; i++)
    ponos::fastSweep2D<ponos::CRegularGrid2D<float>,
                       ponos::RegularGrid2D<CellType>, CellType>(
        &distances, &distances, cell, CellType::FLUID);
}

template <typename ParticleType>
void FLIP2D<ParticleType>::extrapolateVelocity() {
  ponos::CRegularGrid2D<float> *u = grid[CUR_GRID]->v_u.get();
  ponos::CRegularGrid2D<float> *v = grid[CUR_GRID]->v_v.get();
  ponos::RegularGrid2D<CellType> *cell = grid[CUR_GRID]->cellType.get();
  for (int i = 0; i < 4; i++) {
    int nx = u->getDimensions()[0];
    int ny = u->getDimensions()[1];
    ponos::sweep_x<ponos::CRegularGrid2D<float>, ponos::RegularGrid2D<CellType>,
                   CellType>(u, &distances, cell, CellType::AIR, 1, nx - 1, 1,
                             ny - 1);
    ponos::sweep_x<ponos::CRegularGrid2D<float>, ponos::RegularGrid2D<CellType>,
                   CellType>(u, &distances, cell, CellType::AIR, 1, nx - 1,
                             ny - 2, 0);
    ponos::sweep_x<ponos::CRegularGrid2D<float>, ponos::RegularGrid2D<CellType>,
                   CellType>(u, &distances, cell, CellType::AIR, nx - 2, 0, 1,
                             ny - 1);
    ponos::sweep_x<ponos::CRegularGrid2D<float>, ponos::RegularGrid2D<CellType>,
                   CellType>(u, &distances, cell, CellType::AIR, nx - 2, 0,
                             ny - 2, 0);
    for (int k = 0; k < nx; k++) {
      (*u)(k, 0) = (*u)(k, 1);
      (*u)(k, ny - 1) = (*u)(k, ny - 2);
    }
    for (int k = 0; k < ny; k++) {
      (*u)(0, k) = (*u)(1, k);
      (*u)(nx - 1, k) = (*u)(nx - 2, k);
    }
    nx = v->getDimensions()[0];
    ny = v->getDimensions()[1];
    ponos::sweep_y<ponos::CRegularGrid2D<float>, ponos::RegularGrid2D<CellType>,
                   CellType>(v, &distances, cell, CellType::AIR, 1, nx - 1, 1,
                             ny - 1);
    ponos::sweep_y<ponos::CRegularGrid2D<float>, ponos::RegularGrid2D<CellType>,
                   CellType>(v, &distances, cell, CellType::AIR, 1, nx - 1,
                             ny - 2, 0);
    ponos::sweep_y<ponos::CRegularGrid2D<float>, ponos::RegularGrid2D<CellType>,
                   CellType>(v, &distances, cell, CellType::AIR, nx - 2, 0, 1,
                             ny - 1);
    ponos::sweep_y<ponos::CRegularGrid2D<float>, ponos::RegularGrid2D<CellType>,
                   CellType>(v, &distances, cell, CellType::AIR, nx - 2, 0,
                             ny - 2, 0);
    for (int k = 0; k < nx; k++) {
      (*v)(k, 0) = (*v)(k, 1);
      (*v)(k, ny - 1) = (*v)(k, ny - 2);
    }
    for (int k = 0; k < ny; k++) {
      (*v)(0, k) = (*v)(1, k);
      (*v)(nx - 1, k) = (*v)(nx - 2, k);
    }
  }
}

template <typename ParticleType> void FLIP2D<ParticleType>::subtractGrid() {
  ponos::ivec2 ij;
  ponos::CRegularGrid2D<float> &v = *grid[COPY_GRID]->v_v;
  ponos::CRegularGrid2D<float> &u = *grid[COPY_GRID]->v_u;
  FOR_INDICES0_2D(v.getDimensions(), ij)
  v(ij) = (*grid[CUR_GRID]->v_v)(ij)-v(ij);
  FOR_INDICES0_2D(u.getDimensions(), ij)
  u(ij) = (*grid[CUR_GRID]->v_u)(ij)-u(ij);
}

template <typename ParticleType> void FLIP2D<ParticleType>::solvePICFLIP() {
  particleGrid->iterateAll([](ParticleType *p) { p->v_copy = p->velocity; });

  particleGrid->iterateAll([this](ParticleType *p) {
    if (p->type == ParticleTypes::FLUID) {
      // set FLIP velocity
      p->v_copy += grid[COPY_GRID]->sampleVelocity(p->position);
    }
  });
  particleGrid->iterateAll([this](ParticleType *p) {
    if (p->type == ParticleTypes::FLUID) {
      p->velocity = grid[CUR_GRID]->sampleVelocity(p->position);
      // set PIC velocity
      p->velocity =
          (1.f - pic_flip_ratio) * p->velocity + pic_flip_ratio * p->v_copy;
    }
  });
}

template <typename ParticleType> void FLIP2D<ParticleType>::advectParticles() {
  ponos::RegularGrid2D<CellType> &cell = *grid[CUR_GRID]->cellType;
  float timeStep = 0.2 * dt;
  for (int i = 0; i < 5; i++)
    particleGrid->iterateAll([this, cell, timeStep](ParticleType *p) {
      if (p->type == ParticleTypes::FLUID) {
        // ASSERT_FATAL(cell.dSample(p->position.x, p->position.y,
        //                          CellType::SOLID) == CellType::FLUID);
        // COPY_GRID -> FLIP
        // CUR_GRID  -> PIC
        // Runge Kutta 2
        ponos::Point2 newPosition = p->position;
        ponos::vec2 velocity = p->velocity;
        ponos::vec2 newVelocity =
            (1.f - pic_flip_ratio) *
                (velocity + grid[COPY_GRID]->sampleVelocity(newPosition)) +
            pic_flip_ratio * grid[CUR_GRID]->sampleVelocity(newPosition);
        newPosition += 0.5 * timeStep * newVelocity;
        newVelocity =
            (1.f - pic_flip_ratio) *
                (velocity + grid[COPY_GRID]->sampleVelocity(newPosition)) +
            pic_flip_ratio * grid[CUR_GRID]->sampleVelocity(newPosition);
        newPosition += timeStep * newVelocity;

        if (cell.dSample(newPosition.x, newPosition.y, CellType::FLUID) ==
            CellType::SOLID) {
          ponos::Ray2 r(p->position, newVelocity);
          float t = 0.f, t2 = 0.f;
          float normal[3];
          ASSERT_FATAL(ponos::bbox_ray_intersection(
              cell.cellWBox(cell.cell(newPosition)), r, t, t2, normal));

          ponos::Point2 ip = r(t);
          ponos::vec2 v = newPosition - ip;

          if (ponos::dot(ponos::vec2(normal), v.right()) > 0) {
            newPosition = ip + v.right();
            newVelocity = p->velocity.right();
          } else {
            newPosition = ip + v.left();
            newVelocity = p->velocity.left();
          }
          p->velocity = newVelocity;
          p->position = newPosition;
        } else {
          p->position = newPosition;
        }
      }
    });
  particleGrid->update();
}

template <typename ParticleType>
void FLIP2D<ParticleType>::resampleParticles() {
  static ponos::HaltonSequence rng(3);
  ponos::RegularGrid2D<CellType> &cell = *grid[CUR_GRID]->cellType;
  ponos::ivec2 ij;
  std::vector<ponos::Point2> newPositions;
  std::vector<ponos::vec2> newVelocities;
  particleGrid->update();
  FOR_INDICES0_2D(cell.getDimensions(), ij) {
    if (cell(ij) == CellType::FLUID) {
      if (cell(ij + ponos::ivec2(-1, 0)) != CellType::AIR &&
          cell(ij + ponos::ivec2(1, 0)) != CellType::AIR &&
          cell(ij + ponos::ivec2(0, -1)) != CellType::AIR &&
          cell(ij + ponos::ivec2(0, 1)) != CellType::AIR) {
        float directions[4][2] = {
            {.25, .25}, {.25, .75}, {.75, .25}, {.75, .75}};
        for (int i = 0; i < 4; i++) {
          newPositions.push_back(particleGrid->toWorld(
              ponos::Point2(ij[0], ij[1]) +
              ponos::vec2(directions[i][0], directions[i][1]) +
              ponos::vec2(rng.randomFloat() - 0.5f, rng.randomFloat() - 0.5f) *
                  .5f));
          newVelocities.push_back(ponos::vec2(
              particleGrid->gather(ParticleAttribute::VELOCITY_X,
                                   newPositions[newPositions.size() - 1],
                                   1.5f * dx),
              particleGrid->gather(ParticleAttribute::VELOCITY_Y,
                                   newPositions[newPositions.size() - 1],
                                   1.5f * dx)));
        }

        for (typename ZParticleGrid2D<ParticleType>::particle_iterator it =
                 particleGrid->getCell(ij);
             it.next(); ++it) {
          it.particleElement()->active = false;
        }
      }
    }
  }
  particleGrid->update();
  for (size_t i = 0; i < newPositions.size(); i++) {
    FLIPParticle2D *p = particleGrid->add(newPositions[i]);
    p->velocity = newVelocities[i];
    p->type = ParticleTypes::FLUID;
  }
}

template <typename ParticleType>
void FLIP2D<ParticleType>::copyGridVelocities() {
  grid[COPY_GRID]->v_u->copyData(grid[CUR_GRID]->v_u.get());
  grid[COPY_GRID]->v_v->copyData(grid[CUR_GRID]->v_v.get());
}
