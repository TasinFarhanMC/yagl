#pragma once
#include <betr/filesystem.hpp>
#include <betr/string.hpp>

inline betr::String name = "YAGL";

extern betr::fs::path program_dir;
extern betr::fs::path runtime_dir;

inline betr::fs::path get_src_dir() { return betr::fs::absolute(YAGL_SRC_DIR); }
inline betr::fs::path get_program_dir() { return program_dir; }
inline betr::fs::path get_runtime_dir() { return runtime_dir; }

inline betr::fs::path get_log_dir() { return get_program_dir() / "logs"; }
inline betr::fs::path get_shader_dir() { return get_program_dir() / "shaders"; }
inline betr::fs::path get_shader_cache() { return get_runtime_dir() / "shader_cache"; }
