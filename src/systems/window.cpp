#include "window.hpp"

#include <clay.hpp>
#include <glad/gl.h>
#include <meta.hpp>
#include <systems/logger.hpp>

namespace glfw {

Guard init(uvec2 &size, const vec2 &frac, GLFWwindow *&window) {
  glfwSetErrorCallback([](int error, const char *desc) { LOG_ERROR("Window", "GLFW Error ({}): {}", error, desc); });

  if (!glfwInit()) { return Guard {nullptr}; }

  GLFWmonitor *const monitor = glfwGetPrimaryMonitor();
  if (!monitor) {
    LOG_ERROR("Window", "GLFW Primary monitor is null");
    return Guard {nullptr};
  }

  const GLFWvidmode *mode = glfwGetVideoMode(monitor);
  if (!mode) {
    LOG_ERROR("Window", "GLFW Video mode is null");
    return Guard {nullptr};
  }

  if (size.x == 0) { size.x = mode->width * frac.x; };
  if (size.y == 0) { size.y = mode->height * frac.y; };

  ivec2 monitor_pos;
  glfwGetMonitorPos(monitor, &monitor_pos.x, &monitor_pos.y);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(size.x, size.y, meta::name.c_str(), nullptr, nullptr);
  if (!window) {
    LOG_ERROR("Window", "Failed to create GLFW context");
    return Guard {nullptr};
  }
  glfwMakeContextCurrent(window);

  LOG_INFO("Window", "Created Window ({}x{})", size.x, size.y);

  const uvec2 window_pos = {monitor_pos.x + (mode->width - size.x) / 2, monitor_pos.y + (mode->height - size.y) / 2};
  glfwSetWindowPos(window, window_pos.x, window_pos.y);

  LOG_INFO("Window", "Set Window Pos: {}, {}", window_pos.x, window_pos.y);

  glfwSetWindowUserPointer(window, &size);
  glfwSetWindowSizeCallback(window, [](GLFWwindow *win, int width, int height) {
    auto *size_ptr = static_cast<uvec2 *>(glfwGetWindowUserPointer(win));
    if (size_ptr) {
      size_ptr->x = width;
      size_ptr->y = height;
    }

    Clay_SetLayoutDimensions({width / 1.0f, height / 1.0f});
  });

  if (!gladLoadGL(glfwGetProcAddress)) {
    LOG_ERROR("GL", "Failed to initialize GLAD");
    return Guard {nullptr};
  }

  return Guard {window};
}
} // namespace glfw
