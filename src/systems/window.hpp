#pragma once

#include "math.hpp"
#include <GLFW/glfw3.h>
#include <betr/string.hpp>
#include <systems/logger.hpp>

namespace glfw {
extern ivec2 size;
extern ivec2 pos;

struct Guard {
  GLFWwindow *window;

  explicit Guard(GLFWwindow *w) : window(w) {}

  ~Guard() {
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
    LOG_INFO("Window", "Terminated GLFW & Destroyed Window");
  }

  Guard(const Guard &) = delete;
  Guard &operator=(const Guard &) = delete;

  Guard(Guard &&other) noexcept : window(other.window) { other.window = nullptr; }

  Guard &operator=(Guard &&other) noexcept {
    if (this != &other) {
      window = other.window;
      other.window = nullptr;
    }
    return *this;
  }

  operator bool() const { return window != nullptr; }
};

Guard init(const vec2 &percent, GLFWwindow *&window);

void update_cursor_state(GLFWwindow *window);

enum class Mode {
  Windowed,
  Borderless,
  Fullscreen
};

void set_mode(GLFWwindow *window, Mode mode);
} // namespace glfw
