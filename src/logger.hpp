#pragma once
#include <fmt/core.h>

#define __REL_FILE__ (static_cast<const char *>(__FILE__) + sizeof(YAGL_SRC_PATH))

namespace logger {
std::string current_time();

bool start(bool disable, bool console);
void write(const std::string &str);
void close();

#define LOG_INFO(name, fmt_str, ...)                                                                                                                   \
  logger::write(                                                                                                                                       \
      fmt::format(                                                                                                                                     \
          "[" name "] [{}/INFO] "                                                                                                                      \
          "[{}:{}"                                                                                                                                     \
          "]: " fmt_str "\n",                                                                                                                          \
          logger::current_time(), __REL_FILE__, __LINE__, ##__VA_ARGS__                                                                                \
      )                                                                                                                                                \
  )

#define LOG_WARN(name, fmt_str, ...)                                                                                                                   \
  logger::write(                                                                                                                                       \
      fmt::format(                                                                                                                                     \
          "[" name "] [{}/WARN] "                                                                                                                      \
          "[{}:{}"                                                                                                                                     \
          "]: " fmt_str "\n",                                                                                                                          \
          logger::current_time(), __REL_FILE__, __LINE__, ##__VA_ARGS__                                                                                \
      )                                                                                                                                                \
  )

#define LOG_ERROR(name, fmt_str, ...)                                                                                                                  \
  logger::write(                                                                                                                                       \
      fmt::format(                                                                                                                                     \
          "[" name "] [{}/ERROR] "                                                                                                                     \
          "[{}:{}"                                                                                                                                     \
          "]: " fmt_str "\n",                                                                                                                          \
          logger::current_time(), __REL_FILE__, __LINE__, ##__VA_ARGS__                                                                                \
      )                                                                                                                                                \
  )

#define LOG_FALLBACK(name, fmt_str, ...)                                                                                                               \
  fmt::println(                                                                                                                                        \
      "[" name "] [{}/ERROR] "                                                                                                                         \
      "[{}:{}"                                                                                                                                         \
      "]: " fmt_str,                                                                                                                                   \
      logger::current_time(), __REL_FILE__, __LINE__, ##__VA_ARGS__                                                                                    \
  )
} // namespace logger
