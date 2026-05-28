#pragma once
#include <betr/array.hpp>
#include <graphics/list/shader.hpp>

namespace shader {
extern betr::Array<unsigned, count> programs;

inline unsigned get(int id) { return programs[id]; }

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
} // namespace shader
