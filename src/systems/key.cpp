#include "key.hpp"

#include <betr/namespace.hpp>

static constexpr Array<key::State, GLFW_KEY_LAST + 1> fill_none() {
  Array<key::State, GLFW_KEY_LAST + 1> data {};
  data.fill(key::State::None);
  return data;
}

namespace key {
Array<State, GLFW_KEY_LAST + 1> state = fill_none();
Array<char, GLFW_KEY_LAST + 1> mods = {};

void callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  key::state[key] = static_cast<State>(action);
  key::mods[key] = mods;
}
} // namespace key
