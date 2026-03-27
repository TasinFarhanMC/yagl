#include <phc/phc.hpp>

#include "text.hpp"

namespace text {
String *string = nullptr;
int max_size = -1;

Array<key::State, GLFW_KEY_LAST + 1> key_state;
Array<char, GLFW_KEY_LAST + 1> key_mods;

void callback(GLFWwindow *window, unsigned int codepoint) {
  if (codepoint <= 127 && string && string->size() < max_size) { string->push_back(codepoint); }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  text::key_state[key] = static_cast<key::State>(action);
  text::key_mods[key] = mods;
}
} // namespace text
