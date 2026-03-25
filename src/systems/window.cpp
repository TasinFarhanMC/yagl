#include <phc/phc.hpp>

#include "window.hpp"

#include <glad/gl.h>
#include <meta.hpp>
#include <render/clay.hpp>
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

  glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
    float min = std::min(width / 1366.0f, height / 768.0f);
    float m_width = min * 1366.0f;
    float m_height = min * 768.0f;

    // glViewport(std::abs(width - m_width) / 2, std::abs(height - m_height) / 2, m_width, m_height);
    glViewport(0, 0, width, height);
  });

  if (!gladLoadGL(glfwGetProcAddress)) {
    LOG_ERROR("GL", "Failed to initialize GLAD");
    return Guard {nullptr};
  }

  return Guard {window};
}
} // namespace glfw
