#include <phc/phc.hpp>

#include "texture.hpp"
#include <cstring>
#include <graphics/gltypes.hpp>
#include <meta.hpp>
#include <stb_image.h>
#include <systems/logger.hpp>

namespace texture {
betr::Array<Meta, count> metas;
gl::UniformBuffer<Meta> meta_ubo;

static gl::Buffer<u8vec4> tbo_buffer(GL_TEXTURE_BUFFER);
gl::TextureBuffer tbo;

Guard init(bool clean) {
  i32 size = 0;

  stbi_set_flip_vertically_on_load(0);
  for (int i = 0; i < count; i++) {
    const String &link = links[i];
    const Path path = get_texture_path() / link;

    int width = 0, height = 0;
    bool success = stbi_info((get_texture_path() / path).c_str(), &width, &height, nullptr);
    if (!success) {
      LOG_ERROR("Graphics/Texture", "Failed to query texture [{}]", link);
      return (Guard) {false};
    }

    metas[i].size = {width, height};
    metas[i].offset = size;
    size += width * height;
  }

  meta_ubo.init({});
  meta_ubo.update(metas.data(), count);

  tbo_buffer.init();
  tbo_buffer.update(nullptr, size);
  tbo.init(tbo_buffer, GL_R32UI);
  u8vec4 *data = tbo_buffer.map_range(0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

  i32 offset = 0;
  for (int i = 0; i < count; i++) {
    const String &link = links[i];
    const Path path = get_texture_path() / link;

    int width = 0, height = 0, channels = 0;
    unsigned char *pixels = stbi_load((get_texture_path() / path).c_str(), &width, &height, &channels, 4);
    if (!pixels) {
      LOG_ERROR("Graphics/Texture", "Failed to load texture [{}]", link);
      tbo_buffer.unmap();
      return (Guard) {false};
    }

    const int img_size = width * height;
    memcpy(data + offset, pixels, img_size * sizeof(u8vec4));
    stbi_image_free(pixels);
    LOG_INFO("Graphics/Texture", "Loaded texture [{}] ({}x{}, {} channels) at {:#010X}", link, width, height, channels, offset);

    offset += img_size;
  }
  tbo_buffer.unmap();

  return (Guard) {true};
}

void clean() {
  tbo.destroy();
  tbo_buffer.destroy();
  LOG_INFO("Graphics/Texture", "Cleaned up all textures");
}
} // namespace texture
