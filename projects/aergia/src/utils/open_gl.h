#pragma once

#define GLFW_INCLUDE_GLU
// #include <vulkan/vulkan.h>
// #define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <ponos.h>

namespace aergia {

  void glVertex(ponos::Point3 v);
  void glVertex(ponos::Point2 v);
  void glVertex(ponos::vec2 v);

} // aergia namespace
