#include "rect.hpp"
#include <betr/namespace.hpp>

#include <GLFW/glfw3.h>
#include <betr/chrono.hpp>
#include <betr/def.hpp>
#include <betr/filesystem.hpp>
#include <betr/vector.hpp>
#include <clay.hpp>
#include <glad/gl.h>
#include <graphics/shader.hpp>
#include <iostream>
#include <logger.hpp>
#include <lyra/exe_name.hpp>
#include <lyra/lyra.hpp>
#include <math.hpp>
#include <meta.hpp>

namespace meta {
Path program_path = fs::current_path();
Path runtime_path = fs::current_path();
} // namespace meta

Path bin_path;

Path expand_path(std::string const &input) {
  if (input == "%bin") return bin_path;
  return Path(input);
}

uvec2 window_dim = {0, 0};
float ui_scale = 1;

int main(int argc, const char **argv) {
  bool show_help = false;
  bool console = false;
  bool disable_log = false;
  vec2 window_frac = {2.0f / 3, 2.0f / 3};

  lyra::cli cli;

#ifdef __linux__
  bin_path = fs::read_symlink("/proc/self/exe").parent_path();
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
  char buf[PATH_MAX];
  uint32_t size = sizeof(buf);
  if (_NSGetExecutablePath(buf, &size) == 0) { bin_path = Path(buf).parent_path(); }
#elif defined(_WIN32)
#include <windows.h>
  char buf[MAX_PATH];
  GetModuleFileNameA(NULL, buf, MAX_PATH);
  bin_path = Path(buf).parent_path();
#endif
  try {
    // clang-format off
  cli |= lyra::help(show_help)
        | lyra::opt([](String const& s) { meta::program_path = expand_path(s); }, "path")
            ["-p"]["--program"]("Path to program pathectory or %bin to use binary path")
        | lyra::opt([](String const& s) { meta::runtime_path = expand_path(s); }, "path")
            ["-r"]["--runtime"]("Path to runtime pathectory or %bin to use binary path")
        | (lyra::group() 
          | lyra::opt([&](String const& s) { 
              if (!s.empty() && s.back() == '%') {
                window_frac.x = std::stof(s.substr(0, s.size() - 1)) / 100.0f;
                window_dim.x = 0;
              } else {
                window_frac.x = 0;
                window_dim.x = std::stoi(s);
              }
          }, "width")["-w"]["--width"]("Window Width, Use % for screen percentage").required()
          | lyra::opt([&](String const& s) { 
              if (!s.empty() && s.back() == '%') {
                window_frac.y = std::stof(s.substr(0, s.size() - 1)) / 100.0f;
                window_dim.y = 0;
              } else {
                window_frac.y = 0;
                window_dim.y = std::stoi(s);
              }
          }, "height")["-h"]["--height"]("Window Height, Use % for screen percentage").required())
        | lyra::opt(console)["-c"]["--console"]("Print log to console")
        | lyra::opt(disable_log)["-n"]["--no-log"]("Disable logging");
    // clang-format on
  } catch (const std::exception &e) {
    std::cerr << "Unable to Parse Arguments: " << e.what() << std::endl;
    return 1;
  }

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
    meta::program_path = fs::canonical(meta::program_path);
    meta::runtime_path = fs::weakly_canonical(meta::runtime_path);
  } catch (const fs::filesystem_error &e) {
    LOG_FALLBACK("Init/Filesystem", "{}", e.what());
    return 1;
  }

  std::error_code ec;
  fs::create_directories(get_runtime_path(), ec);
  if (ec) {
    LOG_FALLBACK("Init/Filesystem", "Failed to create runtime directory `{}`: {}", get_runtime_path().string(), ec.message());
    return 1;
  }

  if (!logger::start(disable_log, console)) { return 1; }
  const logger::LogGuard log_guard;

  LOG_INFO("Logger", "Logger Initialized");

  glfwSetErrorCallback([](int error, const char *desc) { LOG_ERROR("Window", "GLFW Error ({}): {}", error, desc); });
  if (!glfwInit()) { return 1; }

  GLFWmonitor *const primary_monitor = glfwGetPrimaryMonitor();

  const GLFWvidmode *const vidmode = glfwGetVideoMode(primary_monitor);
  if (window_dim.x == 0) { window_dim.x = vidmode->width * window_frac.x; }
  if (window_dim.y == 0) { window_dim.y = vidmode->height * window_frac.y; }

  ivec2 monitor_pos;
  glfwGetMonitorPos(primary_monitor, &monitor_pos.x, &monitor_pos.y);

  LOG_INFO(
      "Init/CLI", "Parsed Arguments, program: {}, runtime: {}, width: {}, height: {}, console: {}, disable_log: {}", get_program_path().string(),
      get_runtime_path().string(), window_dim.x, window_dim.y, console, disable_log
  );

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(window_dim.x, window_dim.y, meta::name.c_str(), nullptr, nullptr);
  if (!window) {
    LOG_ERROR("Window", "Failed to create GLFW context");
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSetWindowPos(window, monitor_pos.x + (vidmode->width - window_dim.x) / 2, monitor_pos.y + (vidmode->height - window_dim.y) / 2);

  glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int width, int height) {
    window_dim.x = width;
    window_dim.y = height;

    Clay_SetLayoutDimensions({width / ui_scale, height / ui_scale});
  });

  if (!gladLoadGL(glfwGetProcAddress)) {
    LOG_ERROR("GL", "Failed to initialize GLAD");
    return 1;
  }

  if (!shader::init()) { return 1; }

  clay::init(window_dim);

  renderer::rect::init();

  Vector<RectComp> rects = {
      { 0,  0, 10, 10},
      {30, 50, 20,  5}
  };

  TimePoint<HighResClock> end;
  TimePoint<HighResClock> start = HighResClock::now();
  auto passed_time = HighResClock::duration(0);
  float delta_time = 0;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    // ui update
    glClear(GL_COLOR_BUFFER_BIT);
    // render
    renderer::rect::render(rects);
    glfwSwapBuffers(window);

    end = HighResClock::now();
    const HighResClock::duration delta = end - start;
    start = end;
    passed_time += delta;
    delta_time = Duration<float>(delta).count();

    while (passed_time >= meta::TICK_TIME) {
      if (glfwGetKey(window, GLFW_KEY_ESCAPE)) { glfwSetWindowShouldClose(window, true); }
      logger::flush();

      passed_time -= meta::TICK_TIME;
    }
  }

  renderer::rect::clean();
  clay::clean();
  shader::clean();
  glfwTerminate();

  LOG_INFO("Init", "Completed Cleanup, Saving log and exiting");
  return 0;
}
