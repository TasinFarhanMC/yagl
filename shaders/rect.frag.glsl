#version 330 core

#extension GL_ARB_shading_language_packing : require

flat in uint fColor;
out vec4 color;

void main() {
  color = unpackUnorm4x8(fColor);
}
