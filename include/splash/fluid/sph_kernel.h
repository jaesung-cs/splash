#ifndef SPLASH_FLUID_SPH_KERNEL_H_
#define SPLASH_FLUID_SPH_KERNEL_H_

#include <glm/glm.hpp>

namespace splash
{
namespace fluid
{
class SphKernel
{
public:
  SphKernel() = delete;
  SphKernel(float h)
    : h_(h) {}

  virtual ~SphKernel() = default;

  virtual float operator () (const glm::vec3& r) const = 0;
  virtual glm::vec3 grad(const glm::vec3& r) const = 0;

protected:
  static constexpr float pi = 3.1415926535897932384626433832795f;
  float h_ = 0.f; // Support radius
};

class SphKernelPoly6 final : public SphKernel
{
public:
  SphKernelPoly6() = delete;
  SphKernelPoly6(float h)
    : SphKernel(h)
  {
    h2_ = h * h;
    h3_ = h2_ * h;
    h4_ = h2_ * h2_;
  }
  ~SphKernelPoly6() override = default;

  inline float operator () (const glm::vec3& r) const override
  {
    const auto r2 = glm::dot(r, r);
    if (r2 > h2_)
      return 0.f;

    const auto f = (h2_ - r2) / h3_;
    const auto f3 = f * f * f;
    return 315.f / 64.f / pi * f3;
  }

  inline glm::vec3 grad(const glm::vec3& r) const override
  {
    const auto r2 = glm::dot(r, r);
    if (r2 > h2_)
      return glm::vec3(0.f);

    const auto f = (h2_ - r2) / h4_;
    const auto f2 = f * f;
    return -945.f / 32.f / pi * f2 * (r / h_);
  }

private:
  float h2_;
  float h3_;
  float h4_;
};

class SphKernelSpiky final : public SphKernel
{
public:
  SphKernelSpiky() = delete;
  SphKernelSpiky(float h)
    : SphKernel(h)
  {
    h2_ = h * h;
    h4_ = h2_ * h2_;
    h6_ = h4_ * h2_;
  }

  ~SphKernelSpiky() override = default;

  float operator () (const glm::vec3& r) const override
  {
    const auto r2 = glm::dot(r, r);
    if (r2 > h2_)
      return 0.f;

    const auto r1 = std::sqrt(r2);
    const auto f = (h_ - r1) / h2_;
    const auto f3 = f * f * f;
    return 15.f / pi * f3;
  }

  glm::vec3 grad(const glm::vec3& r) const override
  {
    const auto r2 = glm::dot(r, r);
    if (r2 > h2_)
      return glm::vec3(0.f);

    const auto r1 = std::sqrt(r2);
    if (r1 == 0.f)
      return -45.f / pi / h4_ * glm::vec3(1.f, 0.f, 0.f); // Random direction
    return -45.f / pi / h6_ * (h_ - r1) * (h_ - r1) * (r / r1);
  }

private:
  float h2_;
  float h4_;
  float h6_;
};
}
}

#endif // SPLASH_FLUID_SPH_KERNEL_H_
