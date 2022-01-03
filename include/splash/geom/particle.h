#ifndef SPLASH_GEOM_PARTICLE_H_
#define SPLASH_GEOM_PARTICLE_H_

#include <glm/glm.hpp>

namespace splash
{
namespace geom
{
enum class ParticleType : uint32_t
{
  FLUID,
  BOUNDARY,
};

struct Particle
{
  alignas(16) glm::vec3 position;
  ParticleType type;

  alignas(16) glm::vec3 velocity;
  float mass;

  alignas(16) glm::vec3 color;
  float pad1;
};
}
}

#endif // SPLASH_GEOM_PARTICLE_H_
