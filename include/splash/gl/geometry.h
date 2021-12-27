#ifndef SPLASH_GL_GEOMETRY_H_
#define SPLASH_GL_GEOMETRY_H_

#include <vector>

namespace splash
{
namespace gl
{
struct Attribute
{
  uint32_t index = 0;
  uint32_t size = 0;
  uint32_t stride = 0;
  uint32_t offset = 0;

  Attribute() = delete;

  // Stride, offset in count of float
  Attribute(uint32_t index, uint32_t size, uint32_t stride, uint32_t offset)
    : index(index)
    , size(size)
    , stride(stride)
    , offset(offset)
  {}
};

class Geometry
{
public:
  Geometry() = delete;
  Geometry(
    const std::vector<float>& vertex,
    const std::vector<uint32_t>& index,
    std::initializer_list<Attribute> attributes);
  ~Geometry();

  void draw();

private:
  uint32_t vao_ = 0;
  uint32_t vertexBuffer_ = 0;
  uint32_t indexBuffer_ = 0;

  uint32_t indexCount_ = 0;
};
}
}

#endif // SPLASH_GL_GEOMETRY_H_
