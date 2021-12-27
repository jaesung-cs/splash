#ifndef SPLASH_GL_TEXTURE_H_
#define SPLASH_GL_TEXTURE_H_

#include <cstdint>

namespace splash
{
namespace model
{
class Image;
}

namespace gl
{
class Texture
{
public:
  Texture() = delete;
  explicit Texture(const model::Image& image);
  ~Texture();

  auto id() const noexcept { return texture_; }

  void bind(uint32_t index = 0);
  void unbind();

private:
  uint32_t texture_ = 0;
};
}
}

#endif // SPLASH_GL_TEXTURE_H_
