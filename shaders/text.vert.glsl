#version 330 core

#extension GL_ARB_shading_language_420pack : require
#extension GL_ARB_explicit_uniform_location : require

layout(std140, binding = 0) uniform Space {
  vec2 space;
  float space_y;
};

struct Meta {
  ivec2 size;
  int offset;
  uint padding;
};
layout(std140, binding = 1) uniform TexMeta {
  Meta metas[2048];
};

layout(location = 0) in vec2 base_pos;
layout(location = 1) in vec2 pos;
layout(location = 2) in uint count_scale; // count, scale
layout(location = 3) in int offset;
layout(location = 4) in uint color;

layout(location = 0) uniform int font_id;

const int GLYPH_COUNT = 95;

flat out uint fColor;
flat out int fCharOffset;
flat out int fFontOffset;
flat out int fWidth;
out vec2 uv;

void main() {
  uint count = count_scale & 0xFFFFu;
  uint scale = count_scale >> 16;

  gl_Position = vec4((base_pos * vec2(count, metas[font_id].size.y * float(GLYPH_COUNT) / metas[font_id].size.x) * scale + pos) * 2.0 / space - 1.0, 0.0, 1.0);
  gl_Position.y *= space_y;

  fColor = color;
  uv = vec2(base_pos.x * count, base_pos.y * metas[font_id].size.y);
  fCharOffset = offset;
  fFontOffset = metas[font_id].offset;
  fWidth = metas[font_id].size.x;
}
