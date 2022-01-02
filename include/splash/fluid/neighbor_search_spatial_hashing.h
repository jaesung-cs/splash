#ifndef SPLASH_FLUID_NEIGHBOR_SEARCH_SPATIAL_HASHING_H_
#define SPLASH_FLUID_NEIGHBOR_SEARCH_SPATIAL_HASHING_H_

#include <splash/fluid/neighbor_search.h>

#include <glm/glm.hpp>

namespace splash
{
namespace fluid
{
class NeighborSearchSpatialHashing final : public NeighborSearch
{
public:
  NeighborSearchSpatialHashing();
  ~NeighborSearchSpatialHashing() override;

  void computeNeighbors(const geom::Particles& particles, float h) override;

private:
  uint32_t hash3d(const glm::ivec3& p);

  static constexpr uint32_t hashBucketSize_ = 1000000;
};
}
}

#endif // SPLASH_FLUID_NEIGHBOR_SEARCH_SPATIAL_HASHING_H_
