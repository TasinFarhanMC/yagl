#version 330 core

layout(location = 0) in vec2 base_pos;
layout(location = 1) in vec2 pos;
layout(location = 2) in uvec2 scale;

const vec2 dim = vec2(160.0, 90.0);

void main() {
  vec2 world_pos = base_pos * vec2(scale) + pos;
  gl_Position = vec4(world_pos * 2.0 / dim - 1.0, 0.0, 1.0);
}
