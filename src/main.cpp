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

#include <systems/key.hpp>
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

struct UIState {
  int counter = 0;
  bool sidebarOpen = true;
  Clay_Color sidebarColor = {40, 40, 40, 255};
};

UIState appState;

void HandleCounterClick(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData) {
  if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) { appState.counter++; }
}

void HandleSidebarToggle(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData) {
  if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) { appState.sidebarOpen = !appState.sidebarOpen; }
}

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

  TimePoint<HighResClock> end;
  TimePoint<HighResClock> start = HighResClock::now();
  auto passed_time = HighResClock::duration(0);
  float delta_time = 0;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    glfw::update_cursor_state(window);

    Clay_BeginLayout();
    CLAY({
        .id = CLAY_ID("Root"), .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}},
             .backgroundColor = {20, 20, 20, 255}
    }) {

      // 1. REACTIVE SIDEBAR
      if (appState.sidebarOpen) {
        CLAY({
            .id = CLAY_ID("Sidebar"),
            .layout =
                {.sizing = {CLAY_SIZING_FIXED(250), CLAY_SIZING_GROW(0)},
                         .padding = {16, 16, 16, 16},
                         .childGap = 20,
                         .layoutDirection = CLAY_TOP_TO_BOTTOM},
            .backgroundColor = appState.sidebarColor
        }) {
          CLAY_TEXT(
              CLAY_STRING("SETTINGS"), CLAY_TEXT_CONFIG({
                                           .textColor = {255, 255, 255, 255},
                                             .fontSize = 24
          })
          );

          CLAY({
              .id = CLAY_ID("CloseBtn"), .layout = {.padding = {10, 10, 10, 10}},
                   .backgroundColor = {200, 50, 50, 255}
          }) {
            Clay_OnHover(HandleSidebarToggle, 0);
            CLAY_TEXT(
                CLAY_STRING("Close Sidebar"), CLAY_TEXT_CONFIG({
                                                  .textColor = {255, 255, 255, 255},
                                                    .fontSize = 16
            })
            );
          }
        }
      }

      // 2. MAIN CONTENT AREA
      CLAY({
          .id = CLAY_ID("MainContent"),
          .layout = {
                     .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                     .padding = {32, 32, 32, 32},
                     .childGap = 20, // Moved up to match struct order
              .layoutDirection = CLAY_TOP_TO_BOTTOM
          }
      }) {
        if (!appState.sidebarOpen) {
          CLAY({
              .id = CLAY_ID("OpenBtn"), .layout = {.padding = {8, 8, 8, 8}},
                   .backgroundColor = {50, 150, 50, 255}
          }) {
            Clay_OnHover(HandleSidebarToggle, 0);
            CLAY_TEXT(
                CLAY_STRING(">> Open Sidebar"), CLAY_TEXT_CONFIG({
                                                    .textColor = {255, 255, 255, 255},
                                                      .fontSize = 14
            })
            );
          }
        }

        CLAY_TEXT(
            CLAY_STRING("Counter Application"), CLAY_TEXT_CONFIG({
                                                    .textColor = {255, 255, 255, 255},
                                                      .fontSize = 32
        })
        );

        CLAY({
            .id = CLAY_ID("IncrementBtn"),
            .layout = {.padding = {15, 15, 15, 15}},
            .backgroundColor = Clay_PointerOver(CLAY_ID("IncrementBtn")) ? Clay_Color {80, 80, 80, 255}
              : Clay_Color {60, 60, 60, 255}
        }) {
          Clay_OnHover(HandleCounterClick, 0);
          CLAY_TEXT(
              CLAY_STRING("Click to Increment"), CLAY_TEXT_CONFIG({
                                                     .textColor = {255, 255, 255, 255},
                                                       .fontSize = 18
          })
          );
        }
      }
    }

    Clay_RenderCommandArray clay_cmds = Clay_EndLayout();

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

      if (key::had_state(GLFW_KEY_R, key::State::Press)) {
        LOG_INFO("Shader", "Reloading shaders...");
        shader_guard = shader::init(true);

        if (!shader_guard) { LOG_ERROR("Shader", "Failed to reload shaders!"); }
      }

      if (key::had_state(GLFW_KEY_MINUS, key::State::Press, key::State::Repeat)) { clay::update_scale(clay::scale - 0.1); }
      if (key::had_state(GLFW_KEY_EQUAL, key::State::Press, key::State::Repeat) && key::mods[GLFW_KEY_EQUAL] & GLFW_MOD_SHIFT) {
        clay::update_scale(clay::scale + 0.1);
      }

      logger::flush();

      passed_time -= meta::TICK_TIME;
    }
  }

  return 0;
}
