#include <aergia/io/graphics_display.h>
#include <aergia/io/viewport_display.h>

namespace aergia {

ViewportDisplay::ViewportDisplay(int _x, int _y, int _width, int _height)
    : x(_x), y(_y), width(_width), height(_height) {
  renderer.reset(new DisplayRenderer(width, height));
}

void ViewportDisplay::render(const std::function<void(CameraInterface*)> &f) {
  glEnable(GL_DEPTH_TEST);
  renderer->process([&]() {
    if (f)
      f(camera.get());
    if (renderCallback)
      renderCallback(camera.get());
  });
//  glDisable(GL_DEPTH_TEST);
  GraphicsDisplay &gd = GraphicsDisplay::instance();
  glViewport(x, y, width, height);
  glScissor(x, y, width, height);
  glEnable(GL_SCISSOR_TEST);
  gd.clearScreen(0.f, 1.f, 1.f, 0.f);
//  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  renderer->render();
  glDisable(GL_SCISSOR_TEST);
}

void ViewportDisplay::mouse(double x, double y) {
  if (mouseCallback)
    mouseCallback(x, y);
  camera->mouseMove(getMouseNPos());
}

void ViewportDisplay::scroll(double dx, double dy) {
  camera->mouseScroll(getMouseNPos(), ponos::vec2(dx, dy));
}

void ViewportDisplay::button(int b, int a, int m) {
  if (buttonCallback)
    buttonCallback(b, a, m);
  camera->mouseButton(a, b, getMouseNPos());
}

void ViewportDisplay::key(int k, int scancode, int action, int modifiers) {
  if (keyCallback)
    keyCallback(k, scancode, action, modifiers);
}

ponos::Point2 ViewportDisplay::getMouseNPos() {
  int viewport[] = {0, 0, width, height};
  ponos::Point2 mp =
      GraphicsDisplay::instance().getMousePos() - ponos::vec2(x, y);
  return ponos::Point2((mp.x - viewport[0]) / viewport[2] * 2.0 - 1.0,
                       (mp.y - viewport[1]) / viewport[3] * 2.0 - 1.0);
}

bool ViewportDisplay::hasMouseFocus() const {
  ponos::Point2 mp = GraphicsDisplay::instance().getMousePos() - ponos::vec2(x, y);
  return (mp.x >= 0.f && mp.x <= width && mp.y >= 0.f && mp.y <= height);
}

ponos::Point3 ViewportDisplay::viewCoordToNormDevCoord(ponos::Point3 p) {
  float v[] = {0, 0, static_cast<float>(width), static_cast<float>(height)};
  return ponos::Point3((p.x - v[0]) / (v[2] / 2.0) - 1.0,
                       (p.y - v[1]) / (v[3] / 2.0) - 1.0, 2 * p.z - 1.0);
}

ponos::Point3 ViewportDisplay::unProject(const CameraInterface &c, ponos::Point3 p) {
  return ponos::inverse(c.getTransform()) * p;
}

ponos::Point3 ViewportDisplay::unProject() {
  return ponos::inverse(camera->getTransform()) *
      ponos::Point3(getMouseNPos().x, getMouseNPos().y, 0.f);
}

} // namespace aergia
