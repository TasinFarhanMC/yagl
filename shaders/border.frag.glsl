#version 330 core

#extension GL_ARB_shading_language_packing : require

in vec2 uv;
flat in uint fColor;
flat in vec4 fWidth; // x=left, y=top, z=right, w=bottom (passed as absolute coords)

out vec4 color;

void main() {
  vec4 dists = vec4(uv - fWidth.xy, fWidth.zw - uv);
  color = unpackUnorm4x8(fColor) * step(min(min(dists.x, dists.y), min(dists.z, dists.w)), 0.0);
}
