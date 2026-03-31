#include <phc/phc.hpp>

#include "text.hpp"

namespace text {
String *string = nullptr;
int max_size = -1;
Case case_trans = Case::All;
unsigned code_start = ' ';
unsigned code_end = '~';

void callback(GLFWwindow *window, unsigned int codepoint) {
  char c = static_cast<char>(codepoint);

  switch (case_trans) {
  case Case::Upper: c = (c >= 'a' && c <= 'z') ? (c - 32) : c; break;
  case Case::Lower: c = (c >= 'A' && c <= 'Z') ? (c + 32) : c; break;
  case Case::All: break;
  }

  if (!string || string->size() >= max_size || c < code_start || c > code_end) { return; }

  string->push_back(c);
}
} // namespace text
