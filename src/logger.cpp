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
static Path log_path;

namespace logger {

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

void start(bool console) {
  ::console = console;
  log_path = get_log_path() / "latest.log";
  file.open(log_path, std::ios::out | std::ios::trunc);
}

void write_info(const std::string &str) {
  if (console) std::cout << str;
  file << str;
}

void write(const std::string &str) {
  std::cout << str;
  file << str;
}

void close() {
  using namespace std::chrono;
  auto now = system_clock::now();
  std::time_t now_c = system_clock::to_time_t(now);

  std::tm local_tm {};
  local_tm = *std::localtime(&now_c);

  std::ostringstream ss;
  ss << std::put_time(&local_tm, "%Y-%m-%d_%H-%M-%S") << ".zip";
  const Path zip_path = get_log_path() / ss.str();

  LOG_INFO("Logger", "Saving latest.log to `{}`", zip_path.string());

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
