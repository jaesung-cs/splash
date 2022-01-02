#include <splash/fluid/neighbor_search_naive.h>

#include <splash/geom/particles.h>

namespace splash
{
namespace fluid
{
NeighborSearchNaive::NeighborSearchNaive()
  : NeighborSearch()
{
}

NeighborSearchNaive::~NeighborSearchNaive()
{
}

void NeighborSearchNaive::computeNeighbors(const geom::Particles& particles, float h)
{
  const auto n = particles.size();

  for (int i = 0; i < n; i++)
  {
    for (int j = i + 1; j < n; j++)
    {
    }
  }
}
}
}
