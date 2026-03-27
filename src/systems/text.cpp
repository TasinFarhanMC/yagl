#include <phc/phc.hpp>

#include "text.hpp"

namespace text {
betr::String *string = nullptr;
int max_size = -1;

void callback(GLFWwindow *window, unsigned int codepoint) {
  if (codepoint <= 127 && string && string->size() < max_size) { string->push_back(codepoint); }
}
} // namespace text
