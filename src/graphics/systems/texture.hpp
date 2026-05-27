#pragma once
#include <betr/array.hpp>
#include <graphics/gltypes.hpp>
#include <graphics/list/texture.hpp>
#include <math.hpp>

namespace texture {
struct Meta {
  ivec2 size;
  i32 offset;
  u32 padding;
};

extern betr::Array<Meta, count> metas;
extern gl::UniformBuffer<Meta> meta_ubo;
extern gl::TextureBuffer tbo;

inline const Meta &get(int id) { return metas[id]; }

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

Guard init(bool clean = false);
} // namespace texture
