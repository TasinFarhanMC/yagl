#version 330 core

#extension GL_ARB_shading_language_420pack : require

layout(std140, binding = 0) uniform Clay {
  vec2 space;
};

layout(location = 0) in vec2 base_pos;
layout(location = 1) in vec2 pos;
layout(location = 2) in vec2 size;
layout(location = 3) in vec4 color;

flat out vec4 fColor;

void main() {
  vec2 world_pos = base_pos * vec2(size) + pos;
  gl_Position = vec4(world_pos * 2.0 / space - 1.0, 0.0, 1.0);
  fColor = color;
}
