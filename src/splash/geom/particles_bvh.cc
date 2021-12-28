#include <splash/geom/particles_bvh.h>

#include <algorithm>

namespace splash
{
namespace geom
{
namespace
{
uint64_t splitBy3(uint32_t a)
{
  uint64_t x = a & 0x1fffff; // we only look at the first 21 bits
  x = (x | x << 32) & 0x1f00000000ffff; // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
  x = (x | x << 16) & 0x1f0000ff0000ff; // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
  x = (x | x << 8) & 0x100f00f00f00f00f; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
  x = (x | x << 4) & 0x10c30c30c30c30c3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
  x = (x | x << 2) & 0x1249249249249249;
  return x;
}

uint64_t morton(const glm::uvec3& p)
{
  return splitBy3(p.x) | splitBy3(p.y) << 1 | splitBy3(p.z) << 2;
}

uint32_t clz(uint64_t a)
{
  uint64_t x = 0;
  x |= (!!(a & 0xFFFFFFFF00000000)) << 5;
  x |= (!!(a & 0xFFFF0000FFFF0000)) << 4;
  x |= (!!(a & 0xFF00FF00FF00FF00)) << 3;
  x |= (!!(a & 0xF0F0F0F0F0F0F0F0)) << 2;
  x |= (!!(a & 0xCCCCCCCCCCCCCCCC)) << 1;
  x |= (!!(a & 0xAAAAAAAAAAAAAAAA)) << 0;
  return x;
}
}

ParticlesBvh::ParticlesBvh() = default;

ParticlesBvh::~ParticlesBvh() = default;

void ParticlesBvh::construct(const Particles& particles)
{
  particles_ = &particles;
  sorted_.resize(particles.size());

  computeBoundingBox();
  sortByMortonCode();
  computeTreeRanges();
}

void ParticlesBvh::computeBoundingBox()
{
  auto& particles = particles_->data();

  // Compute bounding box of particle positions
  min_ = particles[0].position;
  max_ = particles[0].position;
  for (const auto& particle : particles)
  {
    const auto& p = particle.position;
    min_ = glm::min(min_, p);
    max_ = glm::max(max_, p);
  }
}

void ParticlesBvh::sortByMortonCode()
{
  const auto& particles = *particles_;
  const auto n = particles.size();

  mortons_.resize(n);
  for (int i = 0; i < n; i++)
    mortons_[i] = { mortonCode(particles[i].position), i };

  std::sort(mortons_.begin(), mortons_.end());

  for (int i = 0; i < n; i++)
    sorted_[i] = particles[mortons_[i].second];
}

uint64_t ParticlesBvh::mortonCode(glm::vec3 p)
{
  // Normalize
  p = (p - min_) / (max_ - min_);

  // To integer coordinate
  constexpr uint32_t mortonBits = 21;
  glm::uvec3 n = p * static_cast<float>(1 << mortonBits);
  n = glm::clamp(n, glm::uvec3(0), glm::uvec3((1 << mortonBits) - 1));

  // Bit interleaving
  return morton(n);
}

void ParticlesBvh::computeTreeRanges()
{
  const auto n = particles_->size();
  ranges_.resize(n);

  ranges_[0].left = 0;
  ranges_[0].right = n - 1;

  // TODO: Run parallel loop
  for (int i = 1; i < n - 1; i++)
  {
    const auto mortonPrev = mortons_[i - 1].first;
    const auto mortonCur = mortons_[i].first;
    const auto mortonNext = mortons_[i + 1].first;

    const auto msb0 = clz(mortonPrev ^ mortonCur);
    const auto msb1 = clz(mortonCur ^ mortonNext);

    if (msb0 > msb1)
    {
      // Split left
      auto split = findSplit(0, i);
      ranges_[i] = { split, i };
    }
    else if (msb0 < msb1)
    {
      // Split right
      auto split = findSplit(i, n - 1);
      ranges_[i] = { i, split };
    }
    else
    {
      // Same morton codes
      ranges_[i] = { -1, -1 };
    }
  }
}

int ParticlesBvh::findSplit(int left, int right)
{
  auto mortonLeft = mortons_[left].first;
  auto mortonRight = mortons_[right].first;

  if (mortonLeft == mortonRight)
    return (mortonLeft + mortonRight) >> 1;

  const auto msb = clz(mortonLeft ^ mortonRight);

  int split = -1;
  int mid;
  while (left < right)
  {
    mid = (left + right) >> 1;
    const auto mortonMid = mortons_[mid].first;

    const auto msb0 = clz(mortonLeft ^ mortonMid);
    if (msb == msb0)
    {
      split = mid;
      right = mid - 1;
    }
    else
      left = mid + 1;
  }

  return split;
}

ParticlesBvh::Range ParticlesBvh::findRange(int pivot)
{
  const auto n = mortons_.size();

  const auto mortonPrev = mortons_[pivot - 1].first;
  const auto mortonCur = mortons_[pivot].first;
  const auto mortonNext = mortons_[pivot + 1].first;

  const auto msb0 = clz(mortonPrev ^ mortonCur);
  const auto msb1 = clz(mortonCur ^ mortonNext);

  Range range;

  if (msb0 > msb1)
  {
    int left = 0;
    int right = pivot;
    int split = -1;
    while (left < right)
    {
      const auto mid = (left + right) >> 1;
      const auto mortonMid = mortons_[mid].first;
      const auto msb = clz(mortonCur ^ mortonMid);

      if (msb <= msb0)
      {
        split = mid;
        right = mid - 1;
      }
      else
        left = mid + 1;
    }

    range.left = split;
    range.right = pivot;
  }
  else if (msb0 < msb1)
  {
    int left = pivot;
    int right = n - 1;
    int split = -1;
    while (left < right)
    {
      const auto mid = (left + right) >> 1;
      const auto mortonMid = mortons_[mid].first;
      const auto msb = clz(mortonCur ^ mortonMid);

      if (msb <= msb1)
      {
        split = mid;
        left = mid + 1;
      }
      else
        right = mid - 1;
    }

    range.left = pivot;
    range.right = split;
  }
  else
  {
    // Same morton codes
    range = { -1, -1 };
  }

  return range;
}
}
}
