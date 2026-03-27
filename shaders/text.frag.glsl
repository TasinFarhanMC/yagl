#version 330 core

#extension GL_ARB_shading_language_420pack : require
#extension GL_ARB_shading_language_packing : require

layout(binding = 0) uniform usamplerBuffer chars;
layout(binding = 1) uniform sampler2D font;

flat in uint fColor;
flat in int offset;
in vec2 uv;

out vec4 color;

const uint GLYPH_START = 32u;
const uint GLYPH_COUNT = 59u;

void main() {
  int index = int(uv.x) + offset;
  uint packed_data = texelFetch(chars, index >> 2).r;
  uint char = (packed_data >> ((index & 3) << 3)) & 0xFFu;

  color = unpackUnorm4x8(fColor) * texture(font, vec2((char + fract(uv.x) - GLYPH_START) / GLYPH_COUNT, uv.y));
}
