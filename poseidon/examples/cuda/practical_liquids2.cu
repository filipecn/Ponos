#include "render.h"
#include <lodepng.h>
#include <poseidon/poseidon.h>

using namespace hermes::cuda;
using namespace poseidon::cuda;

inline void flipImage(std::vector<unsigned char> &data, size_t w, size_t h) {
  for (uint x = 0; x < w; x++)
    for (uint y = 0; y < h / 2; y++)
      for (uint k = 0; k < 4; k++) {
        unsigned char tmp = data[4 * w * (h - 1 - y) + 4 * x + k];
        data[4 * w * (h - 1 - y) + 4 * x + k] = data[4 * w * y + 4 * x + k];
        data[4 * w * y + 4 * x + k] = tmp;
      }
}

int main() {
  vec2u res(128);
  vec2f s(1.f / 128);
  circe::SceneApp<> app(800, 800, "", false);
  app.addViewport2D(0, 0, 800, 800);
  app.getCamera<circe::UserCamera2D>(0)->fit(ponos::bbox2::unitBox());
  PracticalLiquidsSolver2D pls;
  pls.setResolution(res);
  pls.setSpacing(s);
  // walls
  int wall_thick = 3;
  pls.rasterSolid(bbox2f(point2f(-1), point2f(wall_thick * s.x, 1.f)), vec2f());
  pls.rasterSolid(bbox2f(point2f((res.x - wall_thick) * s.x, -1), point2f(2)),
                  vec2f());
  pls.rasterSolid(bbox2f(point2f(-1), point2f(1.f, wall_thick * s.y)), vec2f());
  LevelSet2H ls(pls.surfaceLevelSet());
  for (auto e : ls.grid().accessor()) {
    e.value = 1000.f;
    e.value = min(e.value, (e.worldPosition() -
                            point2f(res.x * 0.5 * s.x, res.y * 0.85 * s.y))
                                   .length() -
                               8 * s.x);
    e.value = min(e.value, e.worldPosition().y - 0.4f);
  }
  pls.surfaceLevelSet().copy(ls);
  auto force = pls.forceField();
  fill2(force.v().data(), -9.81f);
  fill2(pls.velocity().v().data(), -0.5f);
  pls.step(0.001f);
  LevelSet2ModelD lsm(pls.surfaceLevelSet());
  LevelSet2ModelD slsm(pls.solidLevelSet());
  slsm.color = circe::Color::Yellow();
  app.scene.add(&lsm);
  app.scene.add(&slsm);
  slsm.update();
  app.renderCallback = [&]() {
    pls.step(1.f / 60.f);
    lsm.update();
  };
  int frame = 0;
  app.viewports[0].renderEndCallback = [&]() {
    std::vector<unsigned char> image;
    size_t w, h;
    app.viewports[0].renderer->currentPixels(image, w, h);
    flipImage(image, w, h);
    char filename[20];
    sprintf(filename, "%d.png", frame++);
    unsigned error =
        lodepng::encode(filename, &image[0], static_cast<unsigned int>(w),
                        static_cast<unsigned int>(h));
    if (error)
      std::cout << "encoder error " << error << ": "
                << lodepng_error_text(error) << std::endl;
  };
  app.run();
  return 0;
}