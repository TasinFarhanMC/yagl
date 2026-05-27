#version 330 core

#extension GL_ARB_shading_language_420pack : require
#extension GL_ARB_shading_language_packing : require

layout(binding = 0) uniform usamplerBuffer texture;
layout(binding = 1) uniform usamplerBuffer chars;

flat in uint fColor;
flat in int fCharOffset;
flat in int fFontOffset;
flat in int fWidth;
in vec2 uv;

out vec4 color;

const uint GLYPH_START = 32u;
const uint GLYPH_COUNT = 95u;

void main() {
  int index = int(uv.x) + fCharOffset;
  uint packed_data = texelFetch(chars, index >> 2).r;
  uint char = (packed_data >> ((index & 3) << 3)) & 0xFFu;

  int tex_index = int(float(fract(uv.x) + char - GLYPH_START) * fWidth / GLYPH_COUNT) + int(uv.y) * fWidth + fFontOffset;
  color = unpackUnorm4x8(fColor) * (texelFetch(texture, tex_index).r & 0xFFu) / 255.0;
}
