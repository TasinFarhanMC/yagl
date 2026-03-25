#version 330 core

#extension GL_ARB_shading_language_420pack : require

layout(std140, binding = 0) uniform Clay {
  vec2 space;
};

layout(location = 0) in vec2 base_pos;
layout(location = 1) in vec2 pos;
layout(location = 2) in vec2 size;
layout(location = 3) in vec4 color;
layout(location = 4) in uvec4 width; // x=left, y=top, z=right, w=bottom

flat out vec4 fColor;
flat out vec4 fWidth;
out vec2 uv;

void main() {
  uv = base_pos * size;
  gl_Position = vec4((uv + pos) * 2.0 / space - 1.0, 0.0, 1.0);

  fColor = color;
  fWidth = vec4(width.xy, size - width.zw);
}
