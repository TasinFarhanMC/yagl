#pragma once
#include <fmt/format.h>

#define __REL_FILE__ (static_cast<const char *>(__FILE__) + sizeof(YAGL_SRC_PATH))

namespace logger {
extern fmt::memory_buffer buffer;

std::string current_time();

bool start(bool disable, bool console);
void flush();
void write(const std::string &str);
void close();

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

struct LogGuard {
  ~LogGuard() {
    logger::flush();
    logger::close();
  }
};
} // namespace logger
