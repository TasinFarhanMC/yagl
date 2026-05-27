#pragma once
#include <betr/array.hpp>
#include <betr/string.hpp>

namespace texture {
inline constexpr int base_index = __COUNTER__;

#define DEFINE_TEXTURE(name, path)                                                                                                                     \
  inline constexpr int name = __COUNTER__ - texture::base_index - 1;                                                                                   \
  inline constexpr const char *get_texture_path_by_id(std::integral_constant<int, name>) { return path; }

DEFINE_TEXTURE(font, "font.png");
DEFINE_TEXTURE(img, "img.png");

inline constexpr int count = __COUNTER__ - 1 - base_index;
extern const betr::Array<betr::String, count> links;
} // namespace texture
