#ifndef SPLASH_MODEL_IMAGE_H_
#define SPLASH_MODEL_IMAGE_H_

#include <cstdint>
#include <vector>

namespace splash
{
namespace model
{
class Image
{
public:
  Image() = delete;
  Image(uint32_t width, uint32_t height, uint32_t channels);
  ~Image();

  auto width() const noexcept { return width_; }
  auto height() const noexcept { return height_; }
  auto channels() const noexcept { return channels_; }
  const auto& pixels() const noexcept { return pixels_; }

  auto& operator () (int x, int y, int channel) { return pixels_[channel + x * channels_ + y * channels_ * width_]; }

private:
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  uint32_t channels_ = 0;
  std::vector<uint8_t> pixels_;
};
}
}

#endif // SPLASH_MODEL_IMAGE_H_
