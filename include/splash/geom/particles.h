#ifndef SPLASH_GEOM_PARTICLES_H_
#define SPLASH_GEOM_PARTICLES_H_

#include <cstdint>
#include <vector>

#include <splash/geom/particle.h>

namespace splash
{
namespace geom
{
class Particles
{
public:
  Particles(uint32_t n = 0);
  ~Particles();

  const auto& radius() const noexcept { return radius_; }
  auto& radius() noexcept { return radius_; }

  const auto size() const noexcept { return static_cast<uint32_t>(particles_.size()); }
  const auto& data() const noexcept { return particles_; }
  auto& data() noexcept { return particles_; }

  const auto& operator [] (int index) const { return particles_[index]; }
  auto& operator [] (int index) { return particles_[index]; }

  void resize(uint32_t n);

private:
  std::vector<Particle> particles_;
  float radius_ = 1.f;
};
}
}

#endif // SPLASH_GEOM_PARTICLES_H_
