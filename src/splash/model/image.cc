#include <splash/model/image.h>

namespace splash
{
namespace model
{
Image::Image(uint32_t width, uint32_t height, uint32_t channels)
  : width_(width)
  , height_(height)
  , channels_(channels)
  , pixels_(width * height * channels, 0)
{
}

Image::~Image()
{
}
}
}
