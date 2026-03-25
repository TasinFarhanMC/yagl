#include "rect.hpp"
#include "graphics/shader.hpp"
#include "math.hpp"
#include <gltypes.hpp>
#include <span>
#include <systems/logger.hpp>

static gl::Buffer<RectComp> vertex(GL_ARRAY_BUFFER);
static gl::Array<vec2> base_vertex(GL_ARRAY_BUFFER);
static gl::VertexArray vao;

namespace renderer {
namespace rect {
  void init() {
    vertex.init();
    base_vertex.init({
        {1.0f, 1.0f},
        {0.0f, 1.0f},
        {0.0f, 0.0f},
        {1.0f, 0.0f}
    });
    vao.init();

    vao.bind();

    vao.add_attrib(base_vertex, 0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void *)0);

    vao.add_attrib(vertex, 1, 2, GL_FLOAT, GL_FALSE, sizeof(RectComp), (void *)offsetof(RectComp, pos));
    vao.set_divisor(1, 1); // Tell GL this attribute advances once per instance

    vao.add_iattrib(vertex, 2, 2, GL_UNSIGNED_BYTE, sizeof(RectComp), (void *)offsetof(RectComp, scale));
    vao.set_divisor(2, 1); // Tell GL this attribute advances once per instance
  }

  void render(const std::span<RectComp> &data) {
    vao.bind();
    vertex.update(data.data(), data.size());

    glUseProgram(shader::get(shader::rect));
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, data.size());
  }

  void clean() {
    vao.destroy();
    vertex.destroy();
    base_vertex.destroy();
  }
} // namespace rect
} // namespace renderer
