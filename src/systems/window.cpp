#include <phc/phc.hpp>

#include "window.hpp"

#include <glad/gl.h>
#include <meta.hpp>
#include <render/clay.hpp>
#include <systems/key.hpp>
#include <systems/logger.hpp>

static uvec2 window_size {};

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
  window_size = size;
  LOG_INFO("Window", "Created Window ({}x{})", size.x, size.y);

  const uvec2 window_pos = {monitor_pos.x + (mode->width - size.x) / 2, monitor_pos.y + (mode->height - size.y) / 2};
  glfwSetWindowPos(window, window_pos.x, window_pos.y);

  LOG_INFO("Window", "Set Window Pos: {}, {}", window_pos.x, window_pos.y);

  glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    clay::update_viewport({width, height});
  });

  {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    clay::update_viewport({width, height});
  }

  {
    vec2 dpy;
    glfwGetWindowContentScale(window, &dpy.x, &dpy.y);
    LOG_INFO("Window", "Screen Dpy Scale: {}x{}", dpy.x, dpy.y);
    clay::update_dpi(dpy);
  }
  glfwSetWindowContentScaleCallback(window, [](GLFWwindow *window, float xscale, float yscale) { clay::update_dpi({xscale, yscale}); });

  glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int width, int height) { window_size = {width, height}; });
  glfwSetKeyCallback(window, key::callback);

  if (!gladLoadGL(glfwGetProcAddress)) {
    LOG_ERROR("GL", "Failed to initialize GLAD");
    return Guard {nullptr};
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  return Guard {window};
}

void update_cursor_state(GLFWwindow *window) {
  double x, y;
  glfwGetCursorPos(window, &x, &y);

  Clay_SetPointerState(
      {(float)x / (clay::dpi.x * clay::scale), (float)(window_size.y - y) / (clay::dpi.y * clay::scale)},
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS
  );
}
} // namespace glfw
