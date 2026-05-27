#version 330 core

#extension GL_ARB_shading_language_420pack : require

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
layout(location = 2) in vec2 size;
layout(location = 3) in int id;

flat out int fWidth;
flat out int fOffset;
out vec2 uv;

void main() {
  fWidth = metas[id].size.x;
  fOffset = metas[id].offset;

  gl_Position = vec4((base_pos * size + pos) * 2.0 / space - 1.0, 0.0, 1.0);
  gl_Position.y *= space_y;

  uv = base_pos * metas[id].size;
}
