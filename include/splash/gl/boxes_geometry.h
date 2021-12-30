#ifndef SPLASH_GL_BOXES_GEOMETRY_H_
#define SPLASH_GL_BOXES_GEOMETRY_H_

#include <vector>

#include <glm/glm.hpp>

namespace splash
{
namespace model
{
struct Box;
}

namespace gl
{
class BoxesGeometry
{
public:
  BoxesGeometry() = delete;
  explicit BoxesGeometry(uint32_t size);
  ~BoxesGeometry();

  void update(const std::vector<model::Box>& boxes, const glm::vec3& color);

  void draw();

private:
  uint32_t size_ = 0;

  uint32_t vao_ = 0;
  uint32_t vertexBuffer_ = 0;
  uint32_t indexBuffer_ = 0;
  uint32_t instanceBuffers_[2] = { 0, }; // Box, color

  uint32_t indexCount_ = 0;
  uint32_t instanceCount_ = 0;
};
}
}

#endif // SPLASH_GL_BOXES_GEOMETRY_H_
