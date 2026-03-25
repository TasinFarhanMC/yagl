#pragma once

#include "clay.h"
#include <math.hpp>

inline vec2 as_vec2(const Clay_Vector2 &v) { return vec2(v.x, v.y); }
inline Clay_Vector2 as_clay2(const vec2 &v) { return Clay_Vector2(v.x, v.y); }

inline u8vec4 clay_col_to_u8(const Clay_Color &c) { return u8vec4(c.r, c.g, c.b, c.a); }

namespace clay {
extern float scale;
extern vec2 dpi;

void clean();

struct Guard {
  bool initialized = false;

  explicit Guard(bool ok) : initialized(ok) {}

  ~Guard() {
    if (initialized) { clean(); }
  }

  Guard(const Guard &) = delete;
  Guard &operator=(const Guard &) = delete;

  Guard(Guard &&other) noexcept : initialized(other.initialized) { other.initialized = false; }

  Guard &operator=(Guard &&other) noexcept {
    if (this != &other) {
      initialized = other.initialized;
      other.initialized = false;
    }
    return *this;
  }

  operator bool() const { return initialized; }
};

Guard init(const uvec2 &size);
void render(const Clay_RenderCommandArray &cmds, const vec2 &draw_size);

void update_viewport(vec2 size);

void update_dpi(vec2 dpi);
void update_scale(float scale);
} // namespace clay
