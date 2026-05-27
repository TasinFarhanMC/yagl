#version 330 core

#extension GL_ARB_shading_language_420pack : require
#extension GL_ARB_shading_language_packing : require

layout(binding = 0) uniform usamplerBuffer texture;

flat in int fWidth;
flat in int fOffset;
in vec2 uv;

out vec4 color;

void main() {
  int index = int(uv.x) + int(uv.y) * fWidth + fOffset;
  color = unpackUnorm4x8(texelFetch(texture, index).r);
}
