#pragma once
#include "list/audio.hpp"
#include <betr/array.hpp>
#include <miniaudio.h>

namespace audio {
extern const betr::Array<betr::String, count> links;
extern betr::Array<ma_sound *, count> sounds;

inline ma_sound *get(int id) { return sounds[id]; }

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
} // namespace audio
