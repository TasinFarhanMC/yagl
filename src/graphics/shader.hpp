#pragma once
#include "shader_list.hpp"
#include <betr/array.hpp>

namespace shader {
extern const betr::Array<betr::String, count> links;
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

Guard init();
} // namespace shader
