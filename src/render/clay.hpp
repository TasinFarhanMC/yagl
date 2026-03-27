#pragma once

#include "clay.h"
#include <math.hpp>
#include <string_view>

inline vec2 as_vec2(const Clay_Vector2 &v) { return vec2(v.x, v.y); }
inline Clay_Vector2 as_clay2(const vec2 &v) { return Clay_Vector2(v.x, v.y); }

inline u8vec4 clay_col_to_u8(const Clay_Color &c) { return u8vec4(c.r, c.g, c.b, c.a); }
inline Clay_String clay_string(const std::string &string) { return {true, (i32)string.size(), string.data()}; }
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
void render(const Clay_RenderCommandArray &cmds);

void update_viewport(vec2 size);

void update_dpi(vec2 dpi);
void update_scale(float scale);

constexpr Clay_ElementId hash_string(Clay_String key, const uint32_t offset, const uint32_t seed) {
  uint32_t hash = 0;
  uint32_t base = seed;

  for (int32_t i = 0; i < key.length; i++) {
    base += key.chars[i];
    base += (base << 10);
    base ^= (base >> 6);
  }

  hash = base;
  hash += offset;
  hash += (hash << 10);
  hash ^= (hash >> 6);

  hash += (hash << 3);
  base += (base << 3);
  hash ^= (hash >> 11);
  base ^= (base >> 11);
  hash += (hash << 15);
  base += (base << 15);
  return (Clay_ElementId) {.id = hash + 1, .offset = offset, .baseId = base + 1, .stringId = key};
}

constexpr Clay_ElementId idi(Clay_String label, const u32 index) { return hash_string(label, index, 0); }
constexpr Clay_ElementId idi(const std::string_view label, const u32 index) {
  return hash_string(Clay_String {true, (i32)label.size(), label.data()}, index, 0);
}

constexpr Clay_ElementId id(Clay_String label) { return idi(label, 0); }
constexpr Clay_ElementId id(const std::string_view label) { return idi(Clay_String {true, (i32)label.size(), label.data()}, 0); }

inline Clay_ElementId idi_local(Clay_String label, const u32 index) { return hash_string(label, index, Clay__GetParentElementId()); }
inline Clay_ElementId idi_local(const std::string_view label, const u32 index) {
  return hash_string(Clay_String {true, (i32)label.size(), label.data()}, index, Clay__GetParentElementId());
}

inline Clay_ElementId id_local(Clay_String label) { return idi(label, Clay__GetParentElementId()); }
inline Clay_ElementId id_local(const std::string_view label) {
  return idi(Clay_String {true, (i32)label.size(), label.data()}, Clay__GetParentElementId());
}

} // namespace clay
