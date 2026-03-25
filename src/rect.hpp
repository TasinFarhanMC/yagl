#pragma once
#include <math.hpp>
#include <span>

struct RectComp {
  vec2 pos;
  u8vec2 scale;

  constexpr RectComp(const vec2 &p, const u8vec2 &s) : pos(p), scale(s) {}
  constexpr RectComp(float x, float y, u8 w, u8 h) : pos(x, y), scale(w, h) {}
};

namespace renderer {
namespace rect {
  void init();
  void render(const std::span<RectComp> &data);
  void clean();
} // namespace rect
} // namespace renderer
