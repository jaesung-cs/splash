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
  void computeNeighborsMultiThreaded(const geom::Particles& particles, float h);
  void computeNeighborsSingleThreaded(const geom::Particles& particles, float h);

  uint32_t hash3d(const glm::ivec3& p);

  static constexpr uint32_t hashBucketSize_ = 1000000;
  std::vector<std::vector<uint32_t>> hashTable_;
  std::vector<std::vector<uint32_t>> neighborsPerParticle_;
};
}
}

#endif // SPLASH_FLUID_NEIGHBOR_SEARCH_SPATIAL_HASHING_H_
