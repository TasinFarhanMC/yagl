#version 330 core

#extension GL_ARB_shading_language_420pack : require
#extension GL_ARB_explicit_uniform_location : require

layout(std140, binding = 0) uniform Clay {
  vec2 space;
};

layout(location = 0) in vec2 base_pos;
layout(location = 1) in vec2 pos;
layout(location = 2) in uint count_scale; // count, scale
layout(location = 3) in uint offset;
layout(location = 4) in uint color;

layout(location = 0) uniform float glyph_height;

flat out uint fColor;
flat out int fOffset;
out vec2 uv;

void main() {
  uint count = count_scale & 0xFFFFu;
  uint scale = (count_scale >> 16) & 0xFFFFu;
  fOffset = int(offset);

  gl_Position = vec4((base_pos * vec2(count, glyph_height) * scale + pos) * 2.0 / space - 1.0, 0.0, 1.0);
  gl_Position.y *= -1.0;

  fColor = color;
  uv = vec2(base_pos.x * count, -base_pos.y);
}
