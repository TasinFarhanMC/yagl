#pragma once
#include <fmt/format.h>

#define __REL_FILE__ (static_cast<const char *>(__FILE__) + sizeof(YAGL_SRC_PATH))

namespace logger {
extern fmt::memory_buffer buffer;

std::string current_time();

void flush();
void close();

struct Guard {
  bool initialized = false;

  explicit Guard(bool ok) : initialized(ok) {}

  ~Guard() {
    if (initialized) { logger::close(); }
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

Guard init(bool disable, bool console);
void write(const std::string &str);

#define LOG_INFO(name, fmt_str, ...)                                                                                                                   \
  fmt::format_to(                                                                                                                                      \
      std::back_inserter(logger::buffer),                                                                                                              \
      "[" name "] [{}/INFO] "                                                                                                                          \
      "[{}:{}"                                                                                                                                         \
      "]: " fmt_str "\n",                                                                                                                              \
      logger::current_time(), __REL_FILE__, __LINE__, ##__VA_ARGS__                                                                                    \
  )

#define LOG_WARN(name, fmt_str, ...)                                                                                                                   \
  fmt::format_to(                                                                                                                                      \
      std::back_inserter(logger::buffer),                                                                                                              \
      "[" name "] [{}/WARN] "                                                                                                                          \
      "[{}:{}"                                                                                                                                         \
      "]: " fmt_str "\n",                                                                                                                              \
      logger::current_time(), __REL_FILE__, __LINE__, ##__VA_ARGS__                                                                                    \
  )

#define LOG_ERROR(name, fmt_str, ...)                                                                                                                  \
  fmt::format_to(                                                                                                                                      \
      std::back_inserter(logger::buffer),                                                                                                              \
      "[" name "] [{}/ERROR] "                                                                                                                         \
      "[{}:{}"                                                                                                                                         \
      "]: " fmt_str "\n",                                                                                                                              \
      logger::current_time(), __REL_FILE__, __LINE__, ##__VA_ARGS__                                                                                    \
  )

#define LOG_FALLBACK(name, fmt_str, ...)                                                                                                               \
  fmt::println(                                                                                                                                        \
      "[" name "] [{}/ERROR] "                                                                                                                         \
      "[{}:{}"                                                                                                                                         \
      "]: " fmt_str,                                                                                                                                   \
      logger::current_time(), __REL_FILE__, __LINE__, ##__VA_ARGS__                                                                                    \
  )

} // namespace logger
