#pragma once
#include <betr/array.hpp>
#include <betr/string.hpp>

namespace shader {
inline constexpr int base_index = __COUNTER__;

#define DEFINE_SHADER(name, path)                                                                                                                      \
  inline constexpr int name = __COUNTER__ - shader::base_index - 1;                                                                                    \
  inline constexpr const char *get_shader_path_by_id(std::integral_constant<int, name>) { return path; }

DEFINE_SHADER(TEST_SHADER, "path");

inline constexpr int count = __COUNTER__ - 1 - base_index;
} // namespace shader
