#version 330 core

#extension GL_ARB_shading_language_420pack : require

layout(std140, binding = 0) uniform Clay {
  vec2 space;
};

layout(location = 0) in vec2 base_pos;
layout(location = 1) in vec2 pos;
layout(location = 2) in uint meta; // count, scale, offset(2 byte)
layout(location = 3) in vec4 color;

flat out vec4 fColor;
flat out int offset;
out vec2 uv;

void main() {
  uint count = meta & 0xFFu;
  uint scale = (meta >> 8) & 0xFFu;
  offset = int((meta >> 16) & 0xFFFFu);

  gl_Position = vec4((base_pos * scale + pos) * 2.0 / space - 1.0, 0.0, 1.0);

  fColor = color;
  uv = vec2(base_pos.x * count, base_pos.y);
}
