#ifndef SPLASH_FLUID_NEIGHBOR_SEARCH_H_
#define SPLASH_FLUID_NEIGHBOR_SEARCH_H_

#include <vector>

#include <splash/fluid/neighbor.h>

namespace splash
{
namespace geom
{
class Particles;
}

namespace fluid
{
class NeighborSearch
{
public:
  NeighborSearch();
  virtual ~NeighborSearch();

  virtual std::vector<Neighbor> computeNeighbors(const geom::Particles& particles) = 0;

private:
};
}
}

#endif // SPLASH_FLUID_NEIGHBOR_SEARCH_H_
