#ifndef SPLASH_SCENE_SCENE_FLUID_H_
#define SPLASH_SCENE_SCENE_FLUID_H_

#include <memory>
#include <chrono>
#include <vector>

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
  void updateParticles(float dt);

  static constexpr uint32_t fluidSideX_ = 16;
  static constexpr uint32_t fluidSideY_ = 16;
  static constexpr uint32_t fluidSideZ_ = 12;
  static constexpr uint32_t fluidCount_ = fluidSideX_ * fluidSideY_ * fluidSideZ_;
  static constexpr uint32_t boundarySide_ = 28;
  static constexpr uint32_t boundaryCount_ = boundarySide_ * boundarySide_ * 6; // 6 sides
  static constexpr uint32_t particleCount_ = fluidCount_ + boundaryCount_;
  std::unique_ptr<geom::Particles> particles_;
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

  // Fluid simulation - constraints
  std::vector<float> incompressibility_;
  std::vector<float> incompressibilityDenoms_;
  std::vector<glm::vec3> incompressibilitySelfGrad_;
  std::vector<float> incompressibilityLambdas_;
  std::vector<glm::vec3> deltaP_;

  // Animation
  float animationTime_ = 0.f;
  std::chrono::high_resolution_clock::time_point lastTime_;

  // Rendering options
  bool animation_ = false;
};
}
}

#endif // SPLASH_SCENE_SCENE_FLUID_H_
