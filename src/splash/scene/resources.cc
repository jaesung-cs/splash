#include <splash/scene/resources.h>

#include <glm/glm.hpp>

#include <splash/model/image.h>
#include <splash/model/camera.h>
#include <splash/gl/texture.h>
#include <splash/gl/geometry.h>

namespace splash
{
namespace scene
{
Resources::Resources()
{
  // Floor texture
  constexpr uint32_t floorPixelSize = 256;
  constexpr uint32_t border = 2;
  model::Image floorImage(floorPixelSize, floorPixelSize, 3);
  for (int i = 0; i < floorPixelSize; i++)
  {
    for (int j = 0; j < floorPixelSize; j++)
    {
      uint32_t color = 192;
      if (i < border || i >= floorPixelSize - border ||
        j < border || j >= floorPixelSize - border)
        color = 16;

      for (int c = 0; c < 3; c++)
        floorImage(i, j, c) = color;
    }
  }

  floorTexture_ = std::make_unique<gl::Texture>(floorImage);

  // Floor geometry
  {
    constexpr float floorLength = 10.f;
    std::vector<float> vertex{
      -floorLength, -floorLength, 0.f, 0.f, 0.f, 1.f, -floorLength, -floorLength,
      floorLength, -floorLength, 0.f, 0.f, 0.f, 1.f, floorLength, -floorLength,
      -floorLength, floorLength, 0.f, 0.f, 0.f, 1.f, -floorLength, floorLength,
      floorLength, floorLength, 0.f, 0.f, 0.f, 1.f, floorLength, floorLength,
    };

    std::vector<uint32_t> index{
      0, 1, 2, 2, 1, 3,
    };

    floorGeometry_ = std::make_unique<gl::Geometry>(
      vertex, index,
      std::initializer_list<gl::Attribute>{
        { 0, 3, 8, 0 },
        { 1, 3, 8, 3 },
        { 2, 2, 8, 6 },
    }
    );
  }

  // Camera
  constexpr float pi = 3.1415926535897932384626433832795f;
  camera_ = std::make_unique<model::Camera>(60.f / 180.f * pi);
}

Resources::~Resources()
{
}
}
}
