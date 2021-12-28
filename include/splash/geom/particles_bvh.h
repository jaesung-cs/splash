#ifndef SPLASH_GEOM_PARTICLES_BVH_H_
#define SPLASH_GEOM_PARTICLES_BVH_H_

#include <glm/glm.hpp>

#include <splash/geom/particles.h>

namespace splash
{
namespace geom
{
class Particles;

class ParticlesBvh
{
public:
  ParticlesBvh();
  ~ParticlesBvh();

  void construct(const Particles& particles);

private:
  struct Range
  {
    int left;
    int right;
  };

  void computeBoundingBox();
  void sortByMortonCode();
  uint64_t mortonCode(glm::vec3 p);
  void computeTreeRanges();
  int findSplit(int left, int right);
  Range findRange(int pivot);

  const Particles* particles_ = nullptr;
  std::vector<std::pair<uint64_t, uint32_t>> mortons_; // Pair of morton and index
  Particles sorted_;

  std::vector<Range> ranges_;

  glm::vec3 min_{ 0.f };
  glm::vec3 max_{ 0.f };
};
}
}

#endif // SPLASH_GEOM_PARTICLES_BVH_H_
