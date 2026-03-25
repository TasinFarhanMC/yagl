#include "shader.hpp"

#include <betr/namespace.hpp>

#include <betr/vector.hpp>
#include <fstream>
#include <glad/gl.h>
#include <iostream>
#include <meta.hpp>
#include <systems/logger.hpp>

static String read_file(const Path &p) {
  std::ifstream in(p);
  if (!in) return {};
  return String((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
};

static bool check_shader(GLuint shader, const String &type, const String &base, const String &path) {
  GLint ok = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (ok) { return true; }

  GLint len = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
  String log;

  if (len > 1) {
    log.resize(len);
    glGetShaderInfoLog(shader, len, nullptr, log.data());
  }

  LOG_ERROR("Shader", "{} shader compile error [{}] -> `{}`:\n{}", type, base, path, log);
  return false;
};

namespace shader {
Array<unsigned, count> programs;

Guard init() {
  std::error_code ec;
  fs::create_directory(get_shader_cache(), ec);
  if (ec) {
    LOG_FALLBACK("Init", "Failed to create shader cache `{}`: {}", get_shader_cache().string(), ec.message());
    return Guard {false};
  }

  LOG_INFO("Shader", "Validated shader cache directory `{}`", get_shader_cache().string());

  for (int i = 0; i < links.size(); i++) {
    const auto &link = links[i];
    const Path cache = get_shader_cache() / link;

    GLuint program = glCreateProgram();
    bool cache_hit = Guard {false};

    if (!fs::exists(cache)) {
      LOG_INFO("Shader", "Cache not found for [{}] compiling instead", link);
      goto CACHE_MISS;
    }

    {
      std::ifstream in(cache, std::ios::binary);
      if (!in) { goto CACHE_MISS; }

      GLenum binaryFormat;
      in.read(reinterpret_cast<char *>(&binaryFormat), sizeof(binaryFormat));

      Vector<char> binary((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
      glProgramBinary(program, binaryFormat, binary.data(), binary.size());

      GLint success = 0;
      glGetProgramiv(program, GL_LINK_STATUS, &success);

      cache_hit = success;
      if (success) {
        LOG_INFO("Shader", "Cache loaded for [{}]", link);
        programs[i] = program;
        continue;
      }

      GLint logLength = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

      String log;
      if (logLength > 0) {
        log.resize(logLength);
        glGetProgramInfoLog(program, logLength, nullptr, log.data());
      }

      LOG_WARN("Shader", "Cache load failed for [{}] compiling instead:\n{}", link, log.data());

      glDeleteProgram(program);
      program = glCreateProgram();
    }

  CACHE_MISS:
    const Path base = get_shader_path() / link;

    Path vert = base;
    vert.replace_extension(".vert.glsl");

    Path frag = base;
    frag.replace_extension(".frag.glsl");

    const String vert_src = read_file(vert);
    const String frag_src = read_file(frag);

    if (vert_src.empty()) {
      LOG_ERROR("Shader", "Failed to read vertex shader [{}] -> `{}`", link, vert.string());
      glDeleteProgram(program);
      return Guard {false};
    }

    if (frag_src.empty()) {
      LOG_ERROR("Shader", "Failed to read fragment shader [{}] -> `{}`", link, frag.string());
      glDeleteProgram(program);
      return Guard {false};
    }

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    const char *vs_ptr = vert_src.c_str();
    const char *fs_ptr = frag_src.c_str();

    glShaderSource(vs, 1, &vs_ptr, nullptr);
    glCompileShader(vs);

    glShaderSource(fs, 1, &fs_ptr, nullptr);
    glCompileShader(fs);

    if (!check_shader(vs, "Vertex", link, vert.string()) || !check_shader(fs, "Fragment", link, frag.string())) {
      glDeleteShader(vs);
      glDeleteShader(fs);
      glDeleteProgram(program);
      return Guard {false};
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glProgramParameteri(program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if (!linked) {
      GLint len = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

      String log;
      if (len > 1) {
        log.resize(len);
        glGetProgramInfoLog(program, len, nullptr, log.data());
      }

      LOG_ERROR("Shader", "Link error [{}]:\n{}", link, log);
      glDeleteProgram(program);
      return Guard {false};
    }

    LOG_INFO("Shader", "Compiled and loaded [{}]", link);
    programs[i] = program;

    GLint binaryLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
    if (binaryLength > 0) {
      Vector<char> binary(binaryLength);
      GLenum binaryFormat;
      glGetProgramBinary(program, binaryLength, nullptr, &binaryFormat, binary.data());

      std::ofstream out(cache, std::ios::binary);
      out.write(reinterpret_cast<char *>(&binaryFormat), sizeof(binaryFormat));
      out.write(binary.data(), binary.size());
    }
  }

  return Guard {true};
}

void clean() {
  for (const GLuint program : programs) { glDeleteProgram(program); }
  LOG_INFO("Shader", "Deleted all shaders");
}
} // namespace shader
