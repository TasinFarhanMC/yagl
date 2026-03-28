#pragma once
#include <math.hpp>

namespace mouse {
enum class State : char {
  Press = GLFW_PRESS,
  Release = GLFW_RELEASE
};

extern vec2 pos;
extern vec2 scroll;

extern betr::Array<State, GLFW_MOUSE_BUTTON_LAST + 1> state;
extern betr::Array<bool, GLFW_MOUSE_BUTTON_LAST + 1> read_state;
extern betr::Array<char, GLFW_MOUSE_BUTTON_LAST + 1> mods;

void pos_callback(GLFWwindow *window, double x, double y);
void scroll_callback(GLFWwindow *window, double x, double y);
void button_callback(GLFWwindow *window, int button, int action, int mods);

template <typename... States> bool has_state(int button, States... states) { return ((mouse::state[button] == states) || ...); }
template <typename... States> bool has_state(int button, int mods, States... states) {
  return ((mouse::state[button] == states) || ...) && mouse::mods[button] & mods;
}

template <typename... States> bool had_state(int button, States... states) {
  const bool result = ((mouse::state[button] == states) || ...) && !mouse::read_state[button];
  mouse::read_state[button] = true;
  return result;
}

template <typename... States> bool had_state(int button, int mods, States... states) {
  const bool result = ((mouse::state[button] == states) || ...) && mouse::mods[button] & mods && !mouse::read_state[button];
  mouse::read_state[button] = true;
  return result;
}
} // namespace mouse
