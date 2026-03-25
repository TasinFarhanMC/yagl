#include <phc/phc.hpp>

#include "clay.hpp"

#include <betr/namespace.hpp>

#include <betr/vector.hpp>
#include <graphics/gltypes.hpp>
#include <systems/logger.hpp>

static char *clay_data;
static Clay_Arena arena;

static clay::Guard init_renderers();
static void clean_renderers();

struct RectVertex {
  Clay_BoundingBox bounding_box;
  u8vec4 color;
};

static gl::Buffer<RectVertex> rect_vertex(GL_ARRAY_BUFFER);
static gl::Array<vec2> rect_base(GL_ARRAY_BUFFER);
static gl::VertexArray rect_vao;

namespace clay {
Guard init(const uvec2 &size) {
  const int clay_data_size = Clay_MinMemorySize();
  clay_data = new char[clay_data_size];
  arena = Clay_CreateArenaWithCapacityAndMemory(clay_data_size, clay_data);

  Clay_Initialize(
      arena, {(float)size.x, (float)size.y},
      {[](Clay_ErrorData error) {
         switch (error.errorType) {
         case CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED: Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2); break;
         case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED:
           Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
           break;
         default: LOG_ERROR("UI/Clay", "{}", error.errorText.chars); return;
         }

         delete[] clay_data;
         const int clay_data_size = Clay_MinMemorySize();
         clay_data = new char[clay_data_size];
         arena = Clay_CreateArenaWithCapacityAndMemory(clay_data_size, clay_data);
       },
       nullptr}
  );

  Clay_SetMeasureTextFunction(
      [](Clay_StringSlice text, Clay_TextElementConfig *config, void *user_data) -> Clay_Dimensions {
        return {0, 0}; // DUMMY
      },
      nullptr
  );

  LOG_INFO("UI", "Clay Initialized");

  return init_renderers();
}

void clean() {
  delete[] clay_data;
  clean_renderers();
  LOG_INFO("UI", "Completed Clay Cleanup");
}

void render(const Clay_RenderCommandArray &cmds, const vec2 &draw_size) {

  rect_vertex.bind();

  rect_vertex.update(nullptr, cmds.length);
  RectVertex *rect_ptr = rect_vertex.map();

  int rect_count = 0;

  for (int i = 0; i < cmds.length; i++) {
    const Clay_RenderCommand &cmd = cmds.internalArray[i];

    switch (cmd.commandType) {
    case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
      rect_ptr[rect_count++] = {cmd.boundingBox, clay_col_to_u8(cmd.renderData.rectangle.backgroundColor)};
      break;
    case CLAY_RENDER_COMMAND_TYPE_NONE:
    case CLAY_RENDER_COMMAND_TYPE_BORDER:
    case CLAY_RENDER_COMMAND_TYPE_TEXT:
    case CLAY_RENDER_COMMAND_TYPE_IMAGE:
    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
    case CLAY_RENDER_COMMAND_TYPE_CUSTOM: break;
    }
  }

  rect_vao.bind();
  glUseProgram(shader::get(shader::rect));
  glUniform2f(0, draw_size.x, draw_size.y);
  glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, rect_count);
} // namespace clay

} // namespace clay

static clay::Guard init_renderers() {
  rect_vao.init();

  rect_base.init({
      {1.0f, 1.0f},
      {0.0f, 1.0f},
      {0.0f, 0.0f},
      {1.0f, 0.0f}
  });
  rect_vao.add_attrib(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void *)0);

  rect_vertex.init();
  rect_vao.add_attrib(1, 2, GL_FLOAT, GL_FALSE, sizeof(RectVertex), (void *)offsetof(RectVertex, bounding_box.x));
  rect_vao.set_divisor(1, 1);

  rect_vao.add_attrib(2, 2, GL_FLOAT, false, sizeof(RectVertex), (void *)offsetof(RectVertex, bounding_box.width));
  rect_vao.set_divisor(2, 1);

  rect_vao.add_attrib(3, 4, GL_UNSIGNED_BYTE, true, sizeof(RectVertex), (void *)offsetof(RectVertex, color));
  rect_vao.set_divisor(3, 1);

  return clay::Guard {true};
}

static void clean_renderers() {
  rect_vao.destroy();
  rect_base.destroy();
  rect_vertex.destroy();
}
