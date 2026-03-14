#pragma once
#include <betr/filesystem.hpp>
#include <betr/string.hpp>

inline betr::String name = "YAGL";

extern betr::Path program_path;
extern betr::Path runtime_path;

inline betr::Path get_src_path() { return betr::fs::absolute(YAGL_SRC_PATH); }

inline betr::Path get_program_path() { return program_path; }
inline betr::Path get_shader_path() { return get_program_path() / "shaders"; }

inline betr::Path get_runtime_path() { return runtime_path; }
inline betr::Path get_log_path() { return get_runtime_path() / "logs"; }
inline betr::Path get_shader_cache() { return get_runtime_path() / "shader_cache"; }
