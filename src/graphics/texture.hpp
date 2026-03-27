#pragma once

#include <betr/string.hpp>
#include <glad/gl.h>
#include <meta.hpp>
#include <stb_image.h>
#include <systems/logger.hpp>

class Texture2D {
  GLuint id = 0;

public:
  enum Flags : unsigned int {
    NONE = 0,
    PIXELATED = 1 << 0,
    INTERPOLATED = 1 << 1,
    CLAMP = 1 << 2,
    REPEAT = 1 << 3
  };

  betr::String path;
  unsigned int flags = NONE;
  int width = 0, height = 0, channels = 0;

  Texture2D(const betr::String &path = "", unsigned int flags = NONE, bool flipY = false) : path(path), flags(flags) {}

  Texture2D(const Texture2D &) = delete;
  Texture2D &operator=(const Texture2D &) = delete;

  operator GLuint() const { return id; }

  bool init(bool flipY = false) { return init(path, flipY); }

  bool init(const std::string &path, bool flipY = false) {
    this->path = path;
    if (!flipY) stbi_set_flip_vertically_on_load(1);

    unsigned char *pixels = stbi_load((get_texture_path() / path).c_str(), &width, &height, &channels, 0);
    if (!pixels) {
      LOG_ERROR("Graphics/Texture", "Failed to load texture [{}]", path);
      return false;
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    GLenum internalFormat, format;
    switch (channels) {
    case 1:
      internalFormat = GL_R8;
      format = GL_RED;
      break;
    case 3:
      internalFormat = GL_RGB8;
      format = GL_RGB;
      break;
    case 4:
      internalFormat = GL_RGBA8;
      format = GL_RGBA;
      break;
    default:
      LOG_ERROR("Graphics/Texture", "Unsupported channel count {} in [{}]", channels, path);
      stbi_image_free(pixels);
      id = 0;
      return false;
    }

    glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Filtering
    if (flags & PIXELATED) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else if (flags & INTERPOLATED) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // Wrapping
    GLenum wrapMode = (flags & CLAMP) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    if (flags & REPEAT) wrapMode = GL_REPEAT;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    // Grayscale swizzle
    // if (channels == 1) {
    //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
    //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
    //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);
    //   LOG_INFO("Graphics/Texture", "Applied grayscale swizzle for [{}]", path);
    // }

    stbi_image_free(pixels);
    LOG_INFO("Graphics/Texture", "Loaded texture [{}] ({}x{}, {} channels)", path, width, height, channels);
    return true;
  }

  void bind(GLuint slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
  }

  void destroy() {
    if (id) {
      glDeleteTextures(1, &id);
      LOG_INFO("Graphics/Texture", "Destroyed texture [{}]", path);
      id = 0;
    }
  }
};

class TextureBuffer {
  GLuint id = 0;
  GLuint buffer;

public:
  operator GLuint() const { return id; }

  void init(GLuint buffer, GLenum format) {
    this->buffer = buffer;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_BUFFER, id);
    glTexBuffer(GL_TEXTURE_BUFFER, format, buffer);
  }

  void bind(GLuint slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_BUFFER, id);
  }

  void destroy() { glDeleteTextures(1, &id); }
};
