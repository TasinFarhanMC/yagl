#include "logger.hpp"

#include <betr/namespace.hpp>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <meta.hpp>
#include <miniz/miniz.h>
#include <sstream>

using namespace logger;

static std::ofstream file;
static bool console = false;
static bool disable = false;
static Path log_path;
static Path zip_path;

namespace logger {
fmt::memory_buffer buffer;

std::string current_time() {
  using namespace std::chrono;
  auto now = system_clock::now();
  std::time_t now_c = system_clock::to_time_t(now);

  std::tm local_tm {};
  local_tm = *std::localtime(&now_c);

  std::ostringstream ss;
  ss << std::put_time(&local_tm, "%H:%M:%S");
  return ss.str();
}

Guard start(bool disable, bool console) {
  ::console = console;
  ::disable = disable;
  if (disable) { return Guard {true}; };

  std::error_code ec;
  fs::create_directory(get_log_path(), ec);
  if (ec) {
    LOG_FALLBACK("Init", "Failed to create log directory `{}`: {}", get_log_path().string(), ec.message());
    return Guard {false};
  }

  try {
    log_path = get_log_path() / "latest.log";
    file.open(log_path, std::ios::out | std::ios::trunc);

    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t now_c = system_clock::to_time_t(now);

    std::tm local_tm {};
    local_tm = *std::localtime(&now_c);

    std::ostringstream ss;
    ss << std::put_time(&local_tm, "%Y-%m-%d_%H-%M-%S") << ".zip";
    zip_path = get_log_path() / ss.str();
    return Guard {true};
  } catch (const std::exception &e) {
    LOG_FALLBACK("Init", "Logger initialization failed: {}", e.what());
    return Guard {false};
  }
}

void flush() {
  if (disable || buffer.size() == 0) { return; }
  if (console) { std::cout.write(buffer.data(), buffer.size()); }
  file.write(buffer.data(), buffer.size());
  buffer.clear();
}

void write(const std::string &str) {
  if (disable) { return; }
  if (console) { std::cout << str; }
  file << str;
}

void close() {
  if (disable) { return; }

  LOG_INFO("Logger", "Saving latest.log to `{}`", zip_path.string());

  flush();
  file.close();
  if (file.fail()) { LOG_FALLBACK("Logger", "Failed to close log file `{}`", log_path.string()); }

  mz_zip_archive zip_archive;
  memset(&zip_archive, 0, sizeof(zip_archive));

  if (!mz_zip_writer_init_file(&zip_archive, zip_path.c_str(), 0)) {
    LOG_FALLBACK("Logger", "Failed to init zip archive");
    return;
  }

  if (!mz_zip_writer_add_file(&zip_archive, "latest.log", log_path.c_str(), NULL, 0, MZ_BEST_COMPRESSION)) {
    LOG_FALLBACK("Logger", "Failed to add file to zip");
  }

  mz_zip_writer_finalize_archive(&zip_archive);
  mz_zip_writer_end(&zip_archive);
}

} // namespace logger
