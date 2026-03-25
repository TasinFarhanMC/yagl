#pragma once

#include <glad/gl.h>
#include <initializer_list>
#include <systems/logger.hpp>

namespace gl {
template <typename T> class Buffer {
  const GLenum type;
  GLuint id = 0;

public:
  Buffer(GLenum type) : type(type) {};

  operator GLuint() const { return this->id; }

  void init(const std::initializer_list<T> data = {}) {
    glGenBuffers(1, &id);
    glBindBuffer(type, id);
    glBufferData(type, data.size() * sizeof(T), data.begin(), GL_DYNAMIC_DRAW);
    LOG_INFO("Graphics/GL/Buffer", "Initialized buffer {} with {} elements", id, data.size());
  }

  void destroy() {
    glDeleteBuffers(1, &id);
    LOG_INFO("Graphics/GL/Buffer", "Deleted buffer {}", id);
  }

  void bind() { glBindBuffer(type, id); }

  T *map() { return static_cast<T *>(glMapBuffer(type, GL_READ_WRITE)); }
  void unmap() { glUnmapBuffer(type); }

  void update(const T *data, int size) { glBufferData(type, sizeof(T) * size, data, GL_DYNAMIC_DRAW); }
};

template <typename T> class Array {
  const GLenum type;
  GLuint id = 0;

public:
  Array(GLenum type) : type(type) {};

  operator GLuint() const { return this->id; }

  void init(const std::initializer_list<T> data) {
    glGenBuffers(1, &id);
    glBindBuffer(type, id);
    glBufferData(type, data.size() * sizeof(T), data.begin(), GL_STATIC_DRAW);
    LOG_INFO("Graphics/GL/Array", "Initialized array buffer {} with {} elements", id, data.size());
  }

  void destroy() {
    glDeleteBuffers(1, &id);
    LOG_INFO("Graphics/GL/Array", "Deleted array buffer {}", id);
  }

  void bind() { glBindBuffer(type, id); }

  T *map() { return static_cast<T *>(glMapBuffer(type, GL_READ_ONLY)); }
  void unmap() { glUnmapBuffer(type); }

  Array(Array &&) = delete;
  Array(const Array &) = delete;
  Array &operator=(Array &&) = delete;
  Array &operator=(const Array &) = delete;
};

class VertexArray {
  GLuint id = 0;

public:
  void init() {
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);
    LOG_INFO("Graphics/GL/VAO", "Created VertexArray {}", id);
  }

  void bind() { glBindVertexArray(id); }

  void add_attrib(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *offset) const {
    glVertexAttribPointer(index, size, type, normalized, stride, offset);
    glEnableVertexAttribArray(index);
  }

  void add_iattrib(GLuint index, GLint size, GLenum type, GLsizei stride, const void *offset) const {
    glVertexAttribIPointer(index, size, type, stride, offset);
    glEnableVertexAttribArray(index);
  }

  void set_divisor(GLuint index, GLuint divisor) const { glVertexAttribDivisor(index, divisor); }

  operator GLuint() const { return id; }

  void destroy() {
    glDeleteVertexArrays(1, &id);
    LOG_INFO("Graphics/GL/VAO", "Deleted VertexArray {}", id);
  }
};

template <typename T> class UniformBuffer {
  GLuint ubo = 0;
  GLuint binding;

public:
  UniformBuffer(GLuint binding, const T &initial) : binding(binding) {
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(T), &initial, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo);
    LOG_INFO("Graphics/GL/UBO", "Created UniformBuffer {} bound to slot {}", ubo, binding);
  }

  void set(const T &value) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), &value);
  }

  T *map() {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    return static_cast<T *>(glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE));
  }

  void unmap() {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glUnmapBuffer(GL_UNIFORM_BUFFER);
  }

  ~UniformBuffer() {
    if (ubo) {
      glDeleteBuffers(1, &ubo);
      LOG_INFO("Graphics/GL/UBO", "Deleted UniformBuffer {}", ubo);
      ubo = 0;
    }
  }
};

} // namespace gl
