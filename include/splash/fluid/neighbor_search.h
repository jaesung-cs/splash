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

  void setMultiprocessing(bool flag)
  {
    multiprocessing_ = flag;
  }

  virtual void computeNeighbors(const geom::Particles& particles, float h) = 0;
  const std::vector<Neighbor>& neighbors() const noexcept { return neighbors_; }

protected:
  bool multiprocessing_ = false;
  std::vector<Neighbor> neighbors_;
};
}
}

#endif // SPLASH_FLUID_NEIGHBOR_SEARCH_H_
