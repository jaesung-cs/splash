#include <splash/gl/texture.h>

#include <glad/glad.h>

#include <splash/model/image.h>

namespace splash
{
namespace gl
{
namespace
{
uint32_t getMipLevels(uint32_t size)
{
  uint32_t mipLevels = 0;
  while (size)
  {
    mipLevels++;
    size >>= 1;
  }
  return mipLevels;
}
}

Texture::Texture(const model::Image& image)
{
  const auto width = image.width();
  const auto height = image.height();
  const auto channels = image.channels();
  const auto& pixels = image.pixels();

  const uint32_t mipLevels = getMipLevels(std::max(width, height));

  GLenum internalFormat = 0;
  GLenum format = 0;
  switch (channels)
  {
  case 1:
    internalFormat = GL_R8;
    format = GL_R;
    break;
  case 2:
    internalFormat = GL_RG8;
    format = GL_RG;
    break;
  case 3:
    internalFormat = GL_RGB8;
    format = GL_RGB;
    break;
  case 4:
    internalFormat = GL_RGBA8;
    format = GL_RGBA;
    break;
  }

  glGenTextures(1, &texture_);

  glBindTexture(GL_TEXTURE_2D, texture_);

  glTexStorage2D(GL_TEXTURE_2D, mipLevels, internalFormat, width, height);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, pixels.data());
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
  glDeleteTextures(1, &texture_);
}

void Texture::bind(uint32_t index)
{
  glActiveTexture(GL_TEXTURE0 + index);
  glBindTexture(GL_TEXTURE_2D, texture_);
}

void Texture::unbind()
{
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0);
}
}
}
