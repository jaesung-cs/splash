#include <splash/geom/particles.h>

namespace splash
{
namespace geom
{
Particles::Particles(uint32_t n)
  : particles_(n)
{
}

Particles::~Particles() = default;

void Particles::resize(uint32_t n)
{
  particles_.resize(n);
}
}
}
