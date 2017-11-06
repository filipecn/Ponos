#include <aergia/io/graphics_display.h>
#include <aergia/io/viewport_display.h>
#include <aergia/utils/open_gl.h>

namespace aergia {

ViewportDisplay::ViewportDisplay(int _x, int _y, int _width, int _height)
    : x(_x), y(_y), width(_width), height(_height) {}

void ViewportDisplay::render() {
  GraphicsDisplay &gd = GraphicsDisplay::instance();
  glViewport(x, y, width, height);
  glScissor(x, y, width, height);
  glEnable(GL_SCISSOR_TEST);
  gd.clearScreen(1.f, 1.f, 1.f, 0.f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  if (camera)
    camera->look();
  if (renderCallback)
    renderCallback();
  glDisable(GL_SCISSOR_TEST);
}

void ViewportDisplay::mouse(double x, double y) {
  if (mouseCallback)
    mouseCallback(x, y);
  trackball.mouseMove(*camera.get(), getMouseNPos());
}

void ViewportDisplay::scroll(double dx, double dy) {
  trackball.mouseScroll(*camera.get(), ponos::vec2(dx, dy));
}

void ViewportDisplay::button(int b, int a) {
  if (buttonCallback)
    buttonCallback(b, a);
  if (a == GLFW_RELEASE)
    trackball.buttonRelease(*camera.get(), b, getMouseNPos());
  else
    trackball.buttonPress(*camera.get(), b, getMouseNPos());
}

void ViewportDisplay::key(int k, int action) {
  if (keyCallback)
    keyCallback(k, action);
}

ponos::Point2 ViewportDisplay::getMouseNPos() {
  int viewport[] = {0, 0, width, height};
  ponos::Point2 mp =
      GraphicsDisplay::instance().getMousePos() - ponos::vec2(x, y);
  return ponos::Point2((mp.x - viewport[0]) / viewport[2] * 2.0 - 1.0,
                       (mp.y - viewport[1]) / viewport[3] * 2.0 - 1.0);
}

ponos::Point3 ViewportDisplay::viewCoordToNormDevCoord(ponos::Point3 p) {
  float v[] = {0, 0, static_cast<float>(width), static_cast<float>(height)};
  return ponos::Point3((p.x - v[0]) / (v[2] / 2.0) - 1.0,
                       (p.y - v[1]) / (v[3] / 2.0) - 1.0, 2 * p.z - 1.0);
}

ponos::Point3 ViewportDisplay::unProject(const Camera &c, ponos::Point3 p) {
  return ponos::inverse(c.getTransform()) * p;
}

ponos::Point3 ViewportDisplay::unProject() {
  return ponos::inverse(camera->getTransform()) *
         ponos::Point3(getMouseNPos().x, getMouseNPos().y, 0.f);
}

} // aergia namespace