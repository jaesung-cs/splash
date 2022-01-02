#ifndef SPLASH_FLUID_NEIGHBOR_SEARCH_NAIVE_H_
#define SPLASH_FLUID_NEIGHBOR_SEARCH_NAIVE_H_

#include <splash/fluid/neighbor_search.h>

namespace splash
{
namespace fluid
{
class NeighborSearchNaive final : public NeighborSearch
{
public:
  NeighborSearchNaive();
  ~NeighborSearchNaive() override;

  void computeNeighbors(const geom::Particles& particles, float h) override;

private:
};
}
}

#endif // SPLASH_FLUID_NEIGHBOR_SEARCH_NAIVE_H_
