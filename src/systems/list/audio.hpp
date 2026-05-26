#pragma once
#include <betr/array.hpp>
#include <betr/string.hpp>

namespace audio {
inline constexpr int base_index = __COUNTER__;

#define DEFINE_AUDIO(name, path)                                                                                                                       \
  inline constexpr int name = __COUNTER__ - audio::base_index - 1;                                                                                     \
  inline constexpr const char *get_audio_path_by_id(std::integral_constant<int, name>) { return path; }

DEFINE_AUDIO(otherside, "otherside.mp3");

inline constexpr int count = __COUNTER__ - 1 - base_index;
extern const betr::Array<betr::String, count> links;
} // namespace audio
