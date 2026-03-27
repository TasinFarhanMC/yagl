#pragma once

#include <betr/string.hpp>
namespace text {
extern betr::String *string;
extern int max_size;

void callback(GLFWwindow *window, unsigned int codepoint);
} // namespace text
