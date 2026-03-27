#pragma once
#include <betr/chrono.hpp>
#include <betr/filesystem.hpp>
#include <betr/string.hpp>

namespace meta {
extern betr::Path program_path;
extern betr::Path runtime_path;

inline constexpr unsigned TICK_RATE = 30;
inline constexpr betr::HighResClock::duration TICK_TIME = betr::HighResClock::duration(betr::Seconds(1)) / TICK_RATE;

inline constexpr betr::String name = "YAGL";

inline constexpr int major_ver = 0;
inline constexpr int minor_ver = 0;
inline constexpr int patch_ver = 1;

inline const betr::String get_version() {
  std::ostringstream ss;
  ss << major_ver << '.' << minor_ver << '.' << patch_ver;
  return betr::String(ss.str());
}
} // namespace meta

inline betr::Path get_src_path() { return betr::fs::absolute(YAGL_SRC_PATH); }

inline betr::Path get_program_path() { return meta::program_path; }
inline betr::Path get_shader_path() { return get_program_path() / "shaders"; }
inline betr::Path get_texture_path() { return get_program_path() / "textures"; }

inline betr::Path get_runtime_path() { return meta::runtime_path; }
inline betr::Path get_log_path() { return get_runtime_path() / "logs"; }
inline betr::Path get_shader_cache() { return get_runtime_path() / "shader_cache"; }
