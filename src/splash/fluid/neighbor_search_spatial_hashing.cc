#include <splash/fluid/neighbor_search_spatial_hashing.h>

#include <set>
#include <functional>

#include <tbb/tbb.h>

#include <splash/geom/particles.h>

namespace splash
{
namespace fluid
{
namespace
{
void forEach(int begin, int end, std::function<void(int)> f)
{
  tbb::parallel_for(tbb::blocked_range<int>(begin, end),
    [&](const tbb::blocked_range<int>& range)
    {
      for (int i = range.begin(); i < range.end(); i++)
        f(i);
    });
}
}

NeighborSearchSpatialHashing::NeighborSearchSpatialHashing()
  : NeighborSearch()
{
}

NeighborSearchSpatialHashing::~NeighborSearchSpatialHashing()
{
}

uint32_t NeighborSearchSpatialHashing::hash3d(const glm::ivec3& p)
{
  glm::uvec3 up = p;
  constexpr uint32_t p1 = 73856093;
  constexpr uint32_t p2 = 19349663;
  constexpr uint32_t p3 = 83492791;
  uint32_t n = (p1 * p.x) ^ (p2 * p.y) ^ (p3 * p.z);
  n %= hashBucketSize_;

  // Allow hash collision
  return n;
}

void NeighborSearchSpatialHashing::computeNeighbors(const geom::Particles& particles, float h)
{
  if (multiprocessing_)
    computeNeighborsMultiThreaded(particles, h);
  else
    computeNeighborsSingleThreaded(particles, h);
}

void NeighborSearchSpatialHashing::computeNeighborsMultiThreaded(const geom::Particles& particles, float h)
{
  neighbors_.clear();

  const auto n = particles.size();

  hashTable_.resize(hashBucketSize_);
  forEach(0, hashBucketSize_, [&](int i)
    {
      hashTable_[i].clear();
    });

  neighborsPerParticle_.resize(n);
  forEach(0, n, [&](int i)
    {
      neighborsPerParticle_[i].clear();
    });

  // Add object indices to hash table, single threaded
  for (int i = 0; i < n; i++)
  {
    auto ipos = glm::ivec3(particles[i].position / h);
    const auto hash = hash3d(ipos);

    hashTable_[hash].push_back(i);
  }

  // Neighbor search
  forEach(0, n, [&](int i)
    {
      auto ipos = glm::ivec3(particles[i].position / h);

      std::set<uint32_t> nearbyHashes;
      for (int dx = -1; dx <= 1; dx++)
      {
        for (int dy = -1; dy <= 1; dy++)
        {
          for (int dz = -1; dz <= 1; dz++)
          {
            const auto nearbyHash = hash3d(ipos + glm::ivec3(dx, dy, dz));
            nearbyHashes.insert(nearbyHash);
          }
        }
      }

      for (auto nearbyHash : nearbyHashes)
      {
        for (auto nearbyIndex : hashTable_[nearbyHash])
        {
          // Particle i and nearbyIndex
          const auto i0 = i;
          const auto i1 = nearbyIndex;
          if (i0 != i1)
          {
            const auto& p0 = particles[i0].position;
            const auto& p1 = particles[i1].position;

            if (glm::dot(p0 - p1, p0 - p1) <= h * h)
            {
              neighborsPerParticle_[i].push_back(i1);
            }
          }
        }
      }
    });

  // Collect neighbors
  for (int i = 0; i < n; i++)
  {
    Neighbor neighbor;
    neighbor.i0 = i;

    for (auto i1 : neighborsPerParticle_[i])
    {
      neighbor.i1 = i1;
      neighbors_.push_back(neighbor);
    }
  }
}

void NeighborSearchSpatialHashing::computeNeighborsSingleThreaded(const geom::Particles& particles, float h)
{
  neighbors_.resize(0);

  const auto n = particles.size();

  // Clear hash table cache
  hashTable_.resize(hashBucketSize_);
  for (int i = 0; i < hashTable_.size(); i++)
    hashTable_[i].clear();

  // Add object indices to hash table
  for (int i = 0; i < n; i++)
  {
    auto ipos = glm::ivec3(particles[i].position / h);
    const auto hash = hash3d(ipos);

    hashTable_[hash].push_back(i);
  }

  // Neighbor search
  for (int i = 0; i < n; i++)
  {
    auto ipos = glm::ivec3(particles[i].position / h);

    std::set<uint32_t> nearbyHashes;
    for (int dx = -1; dx <= 1; dx++)
    {
      for (int dy = -1; dy <= 1; dy++)
      {
        for (int dz = -1; dz <= 1; dz++)
        {
          const auto nearbyHash = hash3d(ipos + glm::ivec3(dx, dy, dz));
          nearbyHashes.insert(nearbyHash);
        }
      }
    }

    for (auto nearbyHash : nearbyHashes)
    {
      for (auto nearbyIndex : hashTable_[nearbyHash])
      {
        // Particle i and nearbyIndex
        const auto i0 = i;
        const auto i1 = nearbyIndex;
        if (i0 != i1)
        {
          const auto& p0 = particles[i0].position;
          const auto& p1 = particles[i1].position;

          if (glm::dot(p0 - p1, p0 - p1) <= h * h)
          {
            Neighbor neighbor;
            neighbor.i0 = i0;
            neighbor.i1 = i1;
            neighbors_.push_back(neighbor);
          }
        }
      }
    }
  }
}
}
}
