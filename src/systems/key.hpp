#pragma once
#include <GLFW/glfw3.h>
#include <betr/array.hpp>

namespace key {
enum class State : char {
  Press = GLFW_PRESS,
  Release = GLFW_RELEASE,
  Repeat = GLFW_REPEAT,
  None
};

extern betr::Array<State, GLFW_KEY_LAST + 1> state;
extern betr::Array<char, GLFW_KEY_LAST + 1> mods;

void callback(GLFWwindow *window, int key, int scancode, int action, int mods);

template <typename... States> bool has_state(int key, States... states) { return ((key::state[key] == states) || ...); }
template <typename... States> bool has_state(int key, int mods, States... states) {
  return ((key::state[key] == states) || ...) && key::mods[key] & mods;
}

template <typename... States> bool had_state(int key, States... states) {
  const bool result = ((key::state[key] == states) || ...);
  key::state[key] = State::None;
  return result;
}

template <typename... States> bool had_state(int key, int mods, States... states) {
  const bool result = ((key::state[key] == states) || ...) && key::mods[key] & mods;
  key::state[key] = State::None;
  return result;
}
} // namespace key
