#pragma once

#include <betr/string.hpp>
#include <systems/key.hpp>

namespace text {
extern betr::String *string;
extern int max_size;

extern betr::Array<key::State, GLFW_KEY_LAST + 1> key_state;
extern betr::Array<char, GLFW_KEY_LAST + 1> key_mods;

void callback(GLFWwindow *window, unsigned int codepoint);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

template <typename... States> bool has_state(int key, States... states) { return ((key::state[key] == states) || ...); }
template <typename... States> bool has_state(int key, int mods, States... states) {
  return ((key::state[key] == states) || ...) && key::mods[key] & mods;
}

template <typename... States> bool had_state(int key, States... states) {
  const bool result = ((text::key_state[key] == states) || ...);
  text::key_state[key] = key::State::None;
  return result;
}

template <typename... States> bool had_state(int key, int mods, States... states) {
  const bool result = ((text::key_state[key] == states) || ...) && text::key_mods[key] & mods;
  text::key_state[key] = key::State::None;
  return result;
}
} // namespace text
