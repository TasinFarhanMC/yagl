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
#include <systems/text.hpp>
#include <systems/window.hpp>

namespace meta {
Path program_path = fs::current_path();
Path runtime_path = fs::current_path();
} // namespace meta

static Path bin_path;
Path expand_path(std::string const &input) {
  if (input == "%bin") return bin_path;
  return Path(input);
}

static GLFWwindow *window;

static Clay_RenderCommandArray build_ui();
void exit_text_box() {
  LOG_INFO("Text", "Exited Text Feild");
  glfwSetKeyCallback(window, key::callback);
  text::string = nullptr;
}

int main(int argc, const char **argv) noexcept {
  bool show_help = false;
  bool console = false;
  bool disable_log = false;
  vec2 window_percent = {2.0f / 3, 2.0f / 3};

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
                window_percent.x = std::stof(s.substr(0, s.size() - 1)) / 100.0f;
                glfw::size.x = 0;
              } else {
                window_percent.x = 0;
                glfw::size.x = std::stoi(s);
              }
          }, "width")["-w"]["--width"]("Window Width, Use % for screen percentage (default: 67%)").required()
          | lyra::opt([&](String const& s) { 
              if (!s.empty() && s.back() == '%') {
                window_percent.y = std::stof(s.substr(0, s.size() - 1)) / 100.0f;
                glfw::size.y = 0;
              } else {
                window_percent.y = 0;
                glfw::size.y = std::stoi(s);
              }
          }, "height")["-h"]["--height"]("Window Height, Use % for screen percentage (default: 67%)").required())
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

  const glfw::Guard glfw_guard = glfw::init(window_percent, window);
  if (!glfw_guard) { return 1; }

  LOG_INFO(
      "Init/CLI", "Parsed Arguments, program: {}, runtime: {}, width: {}, height: {}, console: {}, disable_log: {}", get_program_path().string(),
      get_runtime_path().string(), glfw::size.x, glfw::size.y, console, disable_log
  );

  shader::Guard shader_guard = shader::init();
  if (!shader_guard) { return 1; }

  const clay::Guard clay_guard = clay::init(glfw::size);
  if (!clay_guard) { return 1; }
  Clay__debugViewWidth = 600;

  TimePoint<HighResClock> end;
  TimePoint<HighResClock> start = HighResClock::now();
  auto passed_time = HighResClock::duration(0);
  float delta_time = 0;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    glfw::update_cursor_state(window);

    glClear(GL_COLOR_BUFFER_BIT);

    clay::render(build_ui());

    glfwSwapBuffers(window);

    end = HighResClock::now();
    const HighResClock::duration delta = end - start;
    start = end;
    passed_time += delta;
    delta_time = Duration<float>(delta).count();

    while (passed_time >= meta::TICK_TIME) {
      if (key::had_state(GLFW_KEY_ESCAPE, key::State::Press)) { glfwSetWindowShouldClose(window, true); }

      if (key::had_state(GLFW_KEY_R, key::State::Press)) {
        LOG_INFO("Shader", "Reloading shaders...");
        shader_guard = shader::init(true);

        if (!shader_guard) { LOG_ERROR("Shader", "Failed to reload shaders!"); }
      }

      if (key::had_state(GLFW_KEY_BACKSPACE, GLFW_MOD_SHIFT | GLFW_MOD_CONTROL, key::State::Press)) { clay::update_scale(1.0); }
      if (key::had_state(GLFW_KEY_MINUS, GLFW_MOD_SHIFT | GLFW_MOD_CONTROL, key::State::Press, key::State::Repeat)) {
        clay::update_scale(clay::scale - 0.1);
      }
      if (key::had_state(GLFW_KEY_EQUAL, GLFW_MOD_SHIFT | GLFW_MOD_CONTROL, key::State::Press, key::State::Repeat)) {
        clay::update_scale(clay::scale + 0.1);
      }

      if (key::had_state(GLFW_KEY_Q, key::State::Press)) {
        static bool debug_mode = false;
        debug_mode = !debug_mode;
        Clay_SetDebugModeEnabled(debug_mode);
      }

      if (key::had_state(GLFW_KEY_F11, key::State::Press) || text::had_state(GLFW_KEY_F11, key::State::Press)) {
        static int mode = (int)glfw::Mode::Borderless;
        glfw::set_mode(window, static_cast<glfw::Mode>(mode));
        mode = (mode + 1) % 3;
      }

      if (text::had_state(GLFW_KEY_BACKSPACE, key::State::Press, key::State::Repeat) && !text::string->empty()) { text::string->pop_back(); }
      if (text::had_state(GLFW_KEY_ESCAPE, key::State::Press)) { exit_text_box(); }

      logger::flush();

      passed_time -= meta::TICK_TIME;
    }
  }
  return 0;
}

static Clay_RenderCommandArray build_ui() {
  static String text_input;

  Clay_BeginLayout();
  CLAY({
      .id = clay::id("Root"),
      .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}, .padding = {20, 20, 20, 20}, .childGap = 20},
      .border = {.color = {20, 20, 20, 255}, .width = {10, 20, 30, 40}},
  }) {
    Clay_OnHover(
        [](Clay_ElementId element_id, Clay_PointerData pointer_data, intptr_t user_data) {
          if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) { exit_text_box(); }
        },
        0
    );

    static int box_color = 0;
    const Clay_Color colors[2] = {
        {255,  66, 125, 255},
        {  1, 182,  18, 255}
    };

    const Clay_Color hover_colors[2] = {
        {208,  52, 101, 255},
        {  2, 147,  13, 255}
    };

    CLAY({
        .id = clay::id("Buttons"),
        .layout = {
                   .sizing = {.height = CLAY_SIZING_GROW()},
                   .childGap = 20,
                   .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM
        }
    }) {
      CLAY(
          {.id = clay::id("Box"),
           .layout {.sizing = {CLAY_SIZING_PERCENT(0.5), CLAY_SIZING_PERCENT(0.5)}},
           .backgroundColor = Clay_Hovered() ? hover_colors[box_color] : colors[box_color]}
      ) {}

      CLAY({.id = clay::id("Pink"), .layout = {.sizing = {CLAY_SIZING_FIXED(50), CLAY_SIZING_FIXED(50)}}, .backgroundColor = colors[0]}) {
        Clay_OnHover(
            [](Clay_ElementId element_id, Clay_PointerData pointer_data, intptr_t user_data) {
              if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) { box_color = 0; }
            },
            0
        );
      }

      CLAY({.id = clay::id("Green"), .layout = {.sizing = {CLAY_SIZING_FIXED(50), CLAY_SIZING_FIXED(50)}}, .backgroundColor = colors[1]}) {
        Clay_OnHover(
            [](Clay_ElementId element_id, Clay_PointerData pointer_data, intptr_t user_data) {
              if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) { box_color = 1; }
            },
            0
        );
      }
    }

    Clay_TextElementConfig text_config = {.textColor = colors[box_color], .fontSize = 30, .lineHeight = 60};

    CLAY({
        .id = clay::id("Text feild"),
        .layout {.sizing = {CLAY_SIZING_PERCENT(1), CLAY_SIZING_PERCENT(1)}, .padding = {50, 50, 50, 50}, .childAlignment = {.y = CLAY_ALIGN_Y_TOP}},
        .backgroundColor = {100, 100, 100, 255}
    }) {

      CLAY_TEXT(clay_string(text_input), &text_config);

      Clay_OnHover(
          [](Clay_ElementId element_id, Clay_PointerData pointer_data, intptr_t user_data) {
            if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
              LOG_INFO("Text", "Entered Text Feild");
              glfwSetKeyCallback(window, text::key_callback);
              text::string = &text_input;
            }
          },
          0
      );
    }
  }

  return Clay_EndLayout();
}
