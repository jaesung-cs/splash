#ifndef SPLASH_GEOM_PARTICLE_H_
#define SPLASH_GEOM_PARTICLE_H_

#include <glm/glm.hpp>

namespace splash
{
namespace geom
{
struct Particle
{
  alignas(16) glm::vec3 position;
  float pad0;

  alignas(16) glm::vec3 color;
  float pad1;
};
}
}

#endif // SPLASH_GEOM_PARTICLE_H_
