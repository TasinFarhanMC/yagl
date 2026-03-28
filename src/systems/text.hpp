#pragma once

#include <betr/string.hpp>
#include <systems/key.hpp>

namespace text {
enum class Case : char {
  Upper,
  Lower,
  All
};

extern betr::String *string;
extern int max_size;
extern Case case_trans;

extern void callback(GLFWwindow *window, unsigned int codepoint);
} // namespace text
