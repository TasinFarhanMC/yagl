#include <betr/namespace.hpp>

#include "shader.hpp"
#include <logger.hpp>
#include <meta.hpp>

namespace shader {
bool init() {
  std::error_code ec;
  fs::create_directory(get_shader_cache(), ec);
  if (ec) {
    LOG_FALLBACK("Init", "Failed to create shader cache `{}`: {}", get_shader_cache().string(), ec.message());
    return false;
  }

  LOG_INFO("Shader", "Validated shader cache directory `{}`", get_shader_cache().string());

  return true;
}

void clean() {}
} // namespace shader
