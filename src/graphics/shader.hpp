#pragma once
#include "shader_list.hpp"
#include <betr/array.hpp>

namespace shader {
extern const betr::Array<betr::String, count> links;
extern betr::Array<unsigned, count> programs;

inline unsigned get(int id) { return programs[id]; }

bool init();
void clean();
} // namespace shader
