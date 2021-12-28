#ifndef SPLASH_GL_PARTICLES_GEOMETRY_H_
#define SPLASH_GL_PARTICLES_GEOMETRY_H_

#include <cstdint>

namespace splash
{
namespace geom
{
class Particles;
}

namespace gl
{
class ParticlesGeometry
{
public:
  ParticlesGeometry() = delete;
  explicit ParticlesGeometry(uint32_t n);
  ~ParticlesGeometry();

  void update(const geom::Particles& particles);

  void draw();

private:
  uint32_t particleCount_ = 0;

  // Sphere
  uint32_t indexCount_ = 0;

  uint32_t vao_ = 0;
  uint32_t vertexBuffer_ = 0;
  uint32_t indexBuffer_ = 0;
  uint32_t instanceBuffer_ = 0;
};
}
}

#endif // SPLASH_GL_PARTICLES_GEOMETRY_H_
