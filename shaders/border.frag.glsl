#version 330 core

in vec2 uv;
flat in vec4 fColor;
flat in vec4 fWidth; // x=left, y=top, z=right, w=bottom (passed as absolute coords)

out vec4 color;

void main() {
  vec4 dists = vec4(uv - fWidth.xy, fWidth.zw - uv);
  color = fColor * step(min(min(dists.x, dists.y), min(dists.z, dists.w)), 0.0);
}
