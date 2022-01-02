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

  static constexpr uint32_t particleSide_ = 8;
  static constexpr uint32_t particleCount_ = particleSide_ * particleSide_ * particleSide_;
  std::unique_ptr<geom::Particles> particles_;
  std::unique_ptr<gl::ParticlesGeometry> particlesGeometry_;

  // Fluid simulation
  std::vector<glm::vec3> positions_;
  std::unique_ptr<fluid::NeighborSearch> neighborSearch_;
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
