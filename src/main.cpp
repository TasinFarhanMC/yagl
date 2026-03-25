#include <phc/phc.hpp>

#include <betr/namespace.hpp>

#include <betr/chrono.hpp>
#include <betr/def.hpp>
#include <betr/filesystem.hpp>
#include <betr/vector.hpp>
#include <glad/gl.h>
#include <graphics/shader.hpp>
#include <iostream>
#include <lyra/lyra.hpp>
#include <math.hpp>
#include <meta.hpp>
#include <render/clay.hpp>
#include <systems/logger.hpp>
#include <systems/window.hpp>

namespace meta {
Path program_path = fs::current_path();
Path runtime_path = fs::current_path();
} // namespace meta

Path bin_path;

Path expand_path(std::string const &input) {
  if (input == "%bin") return bin_path;
  return Path(input);
}

uvec2 window_size = {0, 0};
float ui_scale = 1;

int main(int argc, const char **argv) noexcept {
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
            ["-p"]["--program"]("Path to program directory or %bin to use binary path")
        | lyra::opt([](String const& s) { meta::runtime_path = expand_path(s); }, "path")
            ["-r"]["--runtime"]("Path to runtime directory or %bin to use binary path")
        | (lyra::group() 
          | lyra::opt([&](String const& s) { 
              if (!s.empty() && s.back() == '%') {
                window_frac.x = std::stof(s.substr(0, s.size() - 1)) / 100.0f;
                window_size.x = 0;
              } else {
                window_frac.x = 0;
                window_size.x = std::stoi(s);
              }
          }, "width")["-w"]["--width"]("Window Width, Use % for screen percentage").required()
          | lyra::opt([&](String const& s) { 
              if (!s.empty() && s.back() == '%') {
                window_frac.y = std::stof(s.substr(0, s.size() - 1)) / 100.0f;
                window_size.y = 0;
              } else {
                window_frac.y = 0;
                window_size.y = std::stoi(s);
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

  const logger::Guard log_guard = logger::start(disable_log, console);
  if (!log_guard) { return 1; }

  LOG_INFO("Logger", "Logger Initialized");

  GLFWwindow *window;
  const glfw::Guard glfw_guard = glfw::init(window_size, window_frac, window);
  if (!glfw_guard) { return 1; }

  LOG_INFO(
      "Init/CLI", "Parsed Arguments, program: {}, runtime: {}, width: {}, height: {}, console: {}, disable_log: {}", get_program_path().string(),
      get_runtime_path().string(), window_size.x, window_size.y, console, disable_log
  );

  shader::Guard shader_guard = shader::init();
  if (!shader_guard) { return 1; }

  const clay::Guard clay_guard = clay::init(window_size);
  if (!clay_guard) { return 1; }

  static bool trigger_shader_reload = false;
  glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_R && action == GLFW_PRESS) { trigger_shader_reload = true; }
  });

  TimePoint<HighResClock> end;
  TimePoint<HighResClock> start = HighResClock::now();
  auto passed_time = HighResClock::duration(0);
  float delta_time = 0;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    Clay_BeginLayout();

    // Main Wrapper - adapts to screen size
    CLAY({
        .id = CLAY_ID("MainContent"),
        .layout =
            {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                     .padding = {24, 24, 24, 24},
                     .childGap = 16, // Spacing between vertical elements
             .layoutDirection = CLAY_TOP_TO_BOTTOM},
        .backgroundColor = {30, 30, 30, 255},
        .border = {.color = {80, 80, 80, 255}, .width = {2, 2, 2, 2}}
    }) {
      // --- TOP NAV BAR ---
      CLAY({
          .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(60)}, .padding = {16, 0}},
          .backgroundColor = {50, 50, 50, 255},
          .border = {.color = {100, 100, 100, 255}, .width = {0, 0, 0, 4}}  // Bottom border only
      }) {
        // Logo or Title would go here
      }

      // --- MAIN CONTENT AREA (H-Box) ---
      CLAY({
          .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}, .childGap = 16, .layoutDirection = CLAY_LEFT_TO_RIGHT}
      }) {
        // SIDEBAR (Fixed Width)
        CLAY({
            .layout = {.sizing = {CLAY_SIZING_FIXED(200), CLAY_SIZING_GROW(0)}},
            .backgroundColor = {40, 40, 40, 255},
            .border = {.color = {80, 80, 80, 255}, .width = {2, 2, 2, 2}}
        }) {}

        // CONTENT (Takes up remaining space)
        CLAY({
            .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}},
            .backgroundColor = {0, 80, 200, 150},
            .border = {.color = {255, 255, 255, 200}, .width = {1, 1, 1, 1}}
        }) {}
      }
    }

    const Clay_RenderCommandArray clay_cmds = Clay_EndLayout();

    // ui update
    glClear(GL_COLOR_BUFFER_BIT);

    clay::render(clay_cmds, window_size);

    glfwSwapBuffers(window);

    end = HighResClock::now();
    const HighResClock::duration delta = end - start;
    start = end;
    passed_time += delta;
    delta_time = Duration<float>(delta).count();

    while (passed_time >= meta::TICK_TIME) {
      if (glfwGetKey(window, GLFW_KEY_ESCAPE)) { glfwSetWindowShouldClose(window, true); }

      if (trigger_shader_reload) {
        trigger_shader_reload = false;

        LOG_INFO("Shader", "Reloading shaders...");
        shader::clean();
        shader_guard = shader::init();

        if (!shader_guard) {
          LOG_ERROR("Shader", "Failed to reload shaders!");
          glfwSetWindowShouldClose(window, true);
        }
      }

      logger::flush();

      passed_time -= meta::TICK_TIME;
    }
  }

  return 0;
}
