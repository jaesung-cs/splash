#ifndef SPLASH_FLUID_NEIGHBOR_SEARCH_SPATIAL_HASHING_H_
#define SPLASH_FLUID_NEIGHBOR_SEARCH_SPATIAL_HASHING_H_

#include <splash/fluid/neighbor_search.h>

namespace splash
{
namespace fluid
{
class NeighborSearchSpatialHashing final : public NeighborSearch
{
public:
  NeighborSearchSpatialHashing();
  ~NeighborSearchSpatialHashing() override;

  std::vector<Neighbor> computeNeighbors(const geom::Particles& particles);

private:
};
}
}

#endif // SPLASH_FLUID_NEIGHBOR_SEARCH_SPATIAL_HASHING_H_
