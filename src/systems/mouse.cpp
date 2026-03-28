#include "mouse.hpp"

namespace mouse {
vec2 pos = {0, 0};
vec2 scroll = {0, 0};

Array<State, GLFW_MOUSE_BUTTON_LAST + 1> state;
Array<bool, GLFW_MOUSE_BUTTON_LAST + 1> read_state;
Array<char, GLFW_MOUSE_BUTTON_LAST + 1> mods;

void pos_callback(GLFWwindow *window, double x, double y) { pos = {x, y}; }
void scroll_callback(GLFWwindow *window, double x, double y) { scroll = {x, y}; }
void button_callback(GLFWwindow *window, int button, int action, int mods) {
  mouse::state[button] = static_cast<State>(action);
  mouse::read_state[button] = false;
  mouse::mods[button] = mods;
}
} // namespace mouse
