#include <phc/phc.hpp>

#include "window.hpp"

#include <glad/gl.h>
#include <meta.hpp>
#include <render/clay.hpp>
#include <systems/key.hpp>
#include <systems/logger.hpp>
#include <systems/text.hpp>

namespace glfw {
ivec2 size;
ivec2 pos;

Guard init(const vec2 &frac, GLFWwindow *&window) {
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

  pos = {monitor_pos.x + (mode->width - size.x) / 2, monitor_pos.y + (mode->height - size.y) / 2};
  glfwSetWindowPos(window, pos.x, pos.y);

  LOG_INFO("Window", "Set Window Pos: {}, {}", pos.x, pos.y);

  {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    clay::update_viewport({width, height});
    LOG_INFO("Window", "Framebuffer Size: {}x{}", width, height);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
      glViewport(0, 0, width, height);
      clay::update_viewport({width, height});
    });
  }

  {
    vec2 dpy;
    glfwGetWindowContentScale(window, &dpy.x, &dpy.y);
    LOG_INFO("Window", "Screen Dpy Scale: {}x{}", dpy.x, dpy.y);

    clay::update_dpi(dpy);
    glfwSetWindowContentScaleCallback(window, [](GLFWwindow *window, float x, float y) { clay::update_dpi({x, y}); });
  }

  glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int width, int height) { size = {width, height}; });
  glfwSetWindowPosCallback(window, [](GLFWwindow *window, int x, int y) { pos = {x, y}; });

  glfwSetKeyCallback(window, key::callback);
  glfwSetCharCallback(window, text::callback);

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
      {(float)x / (clay::dpi.x * clay::scale), (float)(size.y - y) / (clay::dpi.y * clay::scale)},
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS
  );
}

void set_mode(GLFWwindow *window, Mode mode) {
  static ivec2 old_pos, old_size;
  static bool windowed = true;

  if (mode == Mode::Windowed) {
    glfwSetWindowMonitor(window, nullptr, old_pos.x, old_pos.y, old_size.x, old_size.y, 0);
    glfwSetWindowAttrib(window, GLFW_DECORATED, true);
    windowed = true;
    return;
  }

  int count;
  GLFWmonitor **monitors = glfwGetMonitors(&count);

  GLFWmonitor *monitor = nullptr;
  int max_overlap = 0;

  for (int i = 0; i < count; ++i) {
    int x, y;
    glfwGetMonitorPos(monitors[i], &x, &y);

    const GLFWvidmode *mode = glfwGetVideoMode(monitors[i]);
    int width = mode->width;
    int height = mode->height;

    int overlap_w = std::max(0, std::min(pos.x + size.x, x + width) - std::max(pos.x, x));
    int overlap_h = std::max(0, std::min(pos.y + size.y, y + height) - std::max(pos.y, y));
    int overlap_area = overlap_w * overlap_h;

    if (overlap_area > max_overlap) {
      max_overlap = overlap_area;
      monitor = monitors[i];
    }
  }

  int x, y;
  glfwGetMonitorPos(monitor, &x, &y);
  const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);

  if (windowed) {
    old_pos = pos;
    old_size = size;
    windowed = false;
  }

  switch (mode) {
  case Mode::Borderless:
    glfwSetWindowMonitor(window, nullptr, x, y, vidmode->width, vidmode->height, 0);
    glfwSetWindowAttrib(window, GLFW_DECORATED, false);
    return;
  case Mode::Fullscreen: glfwSetWindowMonitor(window, monitor, x, y, vidmode->width, vidmode->height, vidmode->refreshRate); return;
  default: return;
  }
}
} // namespace glfw
