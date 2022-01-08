#ifndef SPLASH_SCENE_SCENE_FLUID_H_
#define SPLASH_SCENE_SCENE_FLUID_H_

#include <memory>
#include <chrono>
#include <vector>
#include <functional>

#include <glm/glm.hpp>

#include <splash/scene/scene.h>

namespace splash
{
namespace gl
{
class Shaders;
class ParticlesGeometry;
}

namespace geom
{
class Particles;
}

namespace fluid
{
class NeighborSearch;
class SphKernel;
}

namespace scene
{
class Resources;

class SceneFluid final : public Scene
{
public:
  SceneFluid() = delete;
  SceneFluid(Resources* resources, gl::Shaders* shaders);
  ~SceneFluid() override;

  void drawUi() override;
  void draw() override;

private:
  Resources* resources_ = nullptr;
  gl::Shaders* shaders_ = nullptr;

  void initializeParticles();
  void updateFluidParticles();
  void updateParticles(float dt);

  void forEach(int begin, int end, std::function<void(int)> f);

  static constexpr uint32_t maxFluidSide_ = 64;
  static constexpr uint32_t maxFluidCount_ = maxFluidSide_ * maxFluidSide_ * maxFluidSide_;
  static constexpr uint32_t maxParticleCount_ = maxFluidCount_ + (maxFluidSide_ * maxFluidSide_ * 6);

  int fluidSideX_ = 16;
  int fluidSideY_ = 16;
  int fluidSideZ_ = 32;
  uint32_t fluidCount_ = 0;
  uint32_t particleCount_ = 0;
  std::unique_ptr<geom::Particles> particles_;
  std::unique_ptr<geom::Particles> fluidParticles_;
  std::unique_ptr<gl::ParticlesGeometry> particlesGeometry_;

  // Fluid simulation
  std::vector<glm::vec3> positions_;
  std::unique_ptr<fluid::NeighborSearch> neighborSearch_;
  std::vector<std::vector<int>> neighborIndices_;
  std::vector<int> fluidIndices_;
  std::vector<int> boundaryIndices_;
  std::vector<int> toFluidIndex_;
  std::vector<float> density_;
  float rho0_ = 0.f; // Rest density
  float timestepScale_ = 1.f;

  // Fluid simulation - constraints
  std::vector<float> incompressibilityLambdas_;
  std::vector<glm::vec3> deltaP_;

  // Animation
  float animationTime_ = 0.f;
  std::chrono::high_resolution_clock::time_point lastTime_;

  // Rendering options
  bool animation_ = false;
  bool multiprocessing_ = false;
  int timestepScaleLevel_ = 0; // Relates to timestep scale

  std::vector<std::unique_ptr<fluid::SphKernel>> kernels_;
  int kernelIndex_ = 0;
  int gradKernelIndex_ = 1;
  float viscosity_ = 0.02f;

  bool showBoundary_ = false;

  bool wave_ = false;
  float waveSpeed_ = 1.f;
  float waveAnimationTime_ = 0.f;
};
}
}

#endif // SPLASH_SCENE_SCENE_FLUID_H_
