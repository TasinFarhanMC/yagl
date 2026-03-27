#include <phc/phc.hpp>

#include "clay.hpp"

#include <betr/namespace.hpp>

#include <betr/vector.hpp>
#include <graphics/gltypes.hpp>
#include <graphics/texture.hpp>
#include <systems/logger.hpp>

static char *clay_data;
static vec2 frame_size(1.0f);

struct ClayUBO {
  vec2 space;
};

struct RectVertex {
  Clay_BoundingBox bounding_box;
  u8vec4 color;
};

struct BorderVertex {
  u16vec4 width;
};

struct TextVertex {
  vec2 pos;

  struct {
    u8 count;
    u8 scale;
    u16 offset;
  } meta;

  u8vec4 color;
};

static gl::UniformBuffer<ClayUBO> clay_ubo(0);

static gl::Array<vec2> base_vertex(GL_ARRAY_BUFFER);

static gl::VertexArray rect_vao;
static gl::Buffer<RectVertex> rect_vertex(GL_ARRAY_BUFFER);

static gl::VertexArray border_vao;
static gl::Buffer<RectVertex> border_rect_vertex(GL_ARRAY_BUFFER);
static gl::Buffer<BorderVertex> border_vertex(GL_ARRAY_BUFFER);

static gl::VertexArray text_vao;
static gl::Buffer<char> text_chars(GL_TEXTURE_BUFFER);

static gl::Buffer<TextVertex> text_vertex(GL_ARRAY_BUFFER);
static TextureBuffer text_tbo;

static Texture2D text_font("font.png");

static clay::Guard init_renderers() {
  clay_ubo.init({frame_size});

  base_vertex.init({
      {1.0f, 1.0f},
      {0.0f, 1.0f},
      {0.0f, 0.0f},
      {1.0f, 0.0f}
  });
  rect_vertex.init();

  border_vertex.init();
  border_rect_vertex.init();

  text_vertex.init();
  text_chars.init();

  text_tbo.init(text_chars, GL_R32UI);

  if (!text_font.init()) { return clay::Guard {false}; }

  auto setup_rect_attribs = [](const gl::VertexArray &vao) {
    vao.add_attrib(1, 2, GL_FLOAT, false, sizeof(RectVertex), (void *)offsetof(RectVertex, bounding_box.x));
    vao.set_divisor(1, 1);

    vao.add_attrib(2, 2, GL_FLOAT, false, sizeof(RectVertex), (void *)offsetof(RectVertex, bounding_box.width));
    vao.set_divisor(2, 1);

    vao.add_attrib(3, 4, GL_UNSIGNED_BYTE, true, sizeof(RectVertex), (void *)offsetof(RectVertex, color));
    vao.set_divisor(3, 1);
  };

  rect_vao.init();

  base_vertex.bind();
  rect_vao.add_attrib(0, 2, GL_FLOAT, false, sizeof(vec2), (void *)0);

  rect_vertex.bind();
  setup_rect_attribs(rect_vao);

  border_vao.init();

  base_vertex.bind();
  rect_vao.add_attrib(0, 2, GL_FLOAT, false, sizeof(vec2), (void *)0);

  border_rect_vertex.bind();
  setup_rect_attribs(border_vao);

  border_vertex.bind();
  border_vao.add_iattrib(4, 4, GL_UNSIGNED_SHORT, sizeof(BorderVertex), (void *)offsetof(BorderVertex, width));
  border_vao.set_divisor(4, 1);

  text_vao.init();

  base_vertex.bind();
  text_vao.add_attrib(0, 2, GL_FLOAT, false, sizeof(vec2), (void *)0);

  text_vertex.bind();
  text_vao.add_attrib(1, 2, GL_FLOAT, false, sizeof(TextVertex), (void *)offsetof(TextVertex, pos));
  // text_vao.add_iattrib(2, 1, GL_UNSIGNED_INT, sizeof(TextVertex), (void *)offsetof(TextVertex, meta));

  return clay::Guard {true};
}

static void clean_renderers() {
  clay_ubo.destroy();

  base_vertex.destroy();

  rect_vertex.destroy();
  border_vertex.destroy();
  border_rect_vertex.destroy();
  text_vertex.destroy();
  text_chars.destroy();

  text_tbo.destroy();
  text_font.destroy();

  rect_vao.destroy();
  border_vao.destroy();
  text_vao.destroy();
}

namespace clay {
float scale = 1.0f;
vec2 dpi(1.0f);

void update_viewport(vec2 size) {
  frame_size = size / (dpi * scale);

  if (Clay_GetCurrentContext()) {
    Clay_SetLayoutDimensions({frame_size.x, frame_size.y});

    clay_ubo.bind();
    clay_ubo.set({frame_size});
  }
}

void update_dpi(vec2 dpi) {
  frame_size = frame_size * clay::dpi / dpi;
  clay::dpi = dpi;

  if (Clay_GetCurrentContext()) {
    Clay_SetLayoutDimensions({frame_size.x, frame_size.y});

    clay_ubo.bind();
    clay_ubo.set({frame_size});
  }
}
void update_scale(float scale) {
  frame_size = frame_size * clay::scale / scale;
  clay::scale = scale;

  Clay_SetLayoutDimensions({frame_size.x, frame_size.y});

  clay_ubo.bind();
  clay_ubo.set({frame_size});
}

Guard init(const uvec2 &size) {
  const int clay_data_size = Clay_MinMemorySize();
  clay_data = new char[clay_data_size];
  static Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(clay_data_size, clay_data);

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

void render(const Clay_RenderCommandArray &cmds) {
  int rect_count = 0;
  int border_count = 0;

  rect_vertex.bind();
  rect_vertex.update(nullptr, cmds.length);
  RectVertex *rect_ptr = rect_vertex.map_range(0, cmds.length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

  border_rect_vertex.bind();
  border_rect_vertex.update(nullptr, cmds.length);
  RectVertex *border_rect_ptr = border_rect_vertex.map_range(0, cmds.length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

  border_vertex.bind();
  border_vertex.update(nullptr, cmds.length);
  BorderVertex *border_ptr = border_vertex.map_range(0, cmds.length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

  for (int i = 0; i < cmds.length; i++) {
    const Clay_RenderCommand &cmd = cmds.internalArray[i];

    switch (cmd.commandType) {
    case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
      rect_ptr[rect_count++] = {cmd.boundingBox, clay_col_to_u8(cmd.renderData.rectangle.backgroundColor)};
      break;
    case CLAY_RENDER_COMMAND_TYPE_BORDER:
      border_rect_ptr[border_count] = {cmd.boundingBox, clay_col_to_u8(cmd.renderData.rectangle.backgroundColor)};
      border_ptr[border_count] = {
          {cmd.renderData.border.width.left, cmd.renderData.border.width.top, cmd.renderData.border.width.right, cmd.renderData.border.width.bottom}
      };
      border_count++;
      break;
    case CLAY_RENDER_COMMAND_TYPE_TEXT:
    default: break;
    }
  }

  border_vertex.unmap();

  border_rect_vertex.bind();
  border_rect_vertex.unmap();

  rect_vertex.bind();
  rect_vertex.unmap();

  rect_vao.bind();
  glUseProgram(shader::get(shader::rect));
  glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, rect_count);

  border_vao.bind();
  glUseProgram(shader::get(shader::border));
  glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, border_count);
}
} // namespace clay
