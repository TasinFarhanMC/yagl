#include <betr/namespace.hpp>

#include <GLFW/glfw3.h>
#include <betr/filesystem.hpp>
#include <filesystem>
#include <fmt/base.h>
#include <graphics/shader.hpp>
#include <iostream>
#include <logger.hpp>
#include <lyra/exe_name.hpp>
#include <lyra/lyra.hpp>
#include <meta.hpp>

Path program_path = fs::current_path();
Path runtime_path = fs::current_path();
Path bin_path;

Path expand_path(std::string const &input) {
  if (input == "%bin") return bin_path;
  return Path(input);
};

int main(int argc, const char **argv) {
  bool show_help = false;
  bool console = false;
  bool disable_log = false;

#ifdef __linux__
  bin_path = fs::read_symlink("/proc/self/exe").parent_path();
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
  char buf[PATH_MAX];
  uint32_t size = sizeof(buf);
  if (_NSGetExecutablePath(buf, &size) == 0) { Path bin_path = Path(buf).parent_path(); }
#elif defined(_WIN32)
#include <windows.h>
  char buf[MAX_PATH];
  GetModuleFileNameA(NULL, buf, MAX_PATH);
  Path bin_path = Path(buf).parent_path();
#endif

  // clang-format off
  lyra::cli cli = lyra::help(show_help)
        | lyra::opt([](String const& s) { program_path = expand_path(s); }, "path")
            ["-p"]["--program"]("Path to program pathectory or %bin to use binary path")
        | lyra::opt([](String const& s) { runtime_path = expand_path(s); }, "path")
            ["-r"]["--runtime"]("Path to runtime pathectory or %bin to use binary path")
        | lyra::opt(console)["-c"]["--console"]("Print log to console")
        | lyra::opt(disable_log)["-n"]["--no-log"]("Disable logging");
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
    program_path = fs::canonical(program_path);
    runtime_path = fs::weakly_canonical(runtime_path);
  } catch (const fs::filesystem_error &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  std::error_code ec;
  fs::create_directories(get_runtime_path(), ec);
  if (ec) {
    LOG_FALLBACK("Init", "Failed to create runtime directory `{}`: {}", get_runtime_path().string(), ec.message());
    return 1;
  }

  if (!logger::start(disable_log, console)) { return 1; }

  LOG_INFO("Logger", "Logger Initialized");
  LOG_INFO(
      "Init", "Parsed Arguments, program: {}, runtime: {}, console: {}, disable_log: {}", get_program_path().string(), get_runtime_path().string(),
      console, disable_log
  );

  glfwSetErrorCallback([](int error, const char *desc) { LOG_ERROR("Window", "GLFW Error ({}): {}", error, desc); });
  if (!glfwInit()) { return 1; }

  if (!shader::init()) { return 1; }

  shader::clean();
  glfwTerminate();

  LOG_INFO("Init", "Completed Cleanup, Saving log and exiting");
  logger::close();

  return 0;
}
