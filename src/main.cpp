#include <betr/namespace.hpp>

#include <betr/filesystem.hpp>
#include <fmt/base.h>
#include <iostream>
#include <lyra/exe_name.hpp>
#include <lyra/lyra.hpp>
#include <meta.hpp>

Path program_dir = fs::current_path();
Path runtime_dir = fs::current_path();
Path bin_dir;

Path expand_path(std::string const &input) {
  if (input == "%bin") return bin_dir;
  return Path(input);
};

int main(int argc, const char **argv) {
  bool show_help = false;

#ifdef __linux__
  bin_dir = fs::read_symlink("/proc/self/exe").parent_path();
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
  char buf[PATH_MAX];
  uint32_t size = sizeof(buf);
  if (_NSGetExecutablePath(buf, &size) == 0) { fs::path bin_dir = fs::path(buf).parent_path(); }
#elif defined(_WIN32)
#include <windows.h>
  char buf[MAX_PATH];
  GetModuleFileNameA(NULL, buf, MAX_PATH);
  fs::path bin_dir = fs::path(buf).parent_path();
#endif

  // clang-format off
  lyra::cli cli = lyra::help(show_help)
        | lyra::opt([](String const& s) { program_dir = expand_path(s); }, "path")
            ["-p"]["--program_dir"]("Path to program directory or %bin to use binary path")
        | lyra::opt([](String const& s) { runtime_dir = expand_path(s); }, "path")
            ["-r"]["--runtime_dir"]("Path to runtime directory or %bin to use binary path");
  // clang-format on

  lyra::parse_result result = cli.parse({argc, argv});

  if (!result) {
    std::cout << cli << std::endl;
    std::cerr << "Error: " << result.message() << std::endl;
    return 1;
  }

  if (show_help) {
    std::cout << cli << std::endl;
    return 0;
  }

  try {
    program_dir = fs::canonical(program_dir);
    runtime_dir = fs::weakly_canonical(runtime_dir);
  } catch (const fs::filesystem_error &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  fmt::println("program_dir: {}", program_dir.string());
  fmt::println("runtime_dir: {}", runtime_dir.string());

  fmt::println("src_dir: {}", get_src_dir().string());
  fmt::println("log_dir: {}", get_log_dir().string());
  fmt::println("shader_dir: {}", get_shader_dir().string());
  fmt::println("shader_cache: {}", get_shader_cache().string());

  return 0;
}
