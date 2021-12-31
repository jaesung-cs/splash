#include <splash/fluid/neighbor_search_spatial_hashing.h>

#include <splash/geom/particles.h>

namespace splash
{
namespace fluid
{
NeighborSearchSpatialHashing::NeighborSearchSpatialHashing()
  : NeighborSearch()
{
}

NeighborSearchSpatialHashing::~NeighborSearchSpatialHashing()
{
}

std::vector<Neighbor> NeighborSearchSpatialHashing::computeNeighbors(const geom::Particles& particles)
{
  // TODO
  return {};
}
}
}
