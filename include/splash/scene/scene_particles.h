#ifndef SPLASH_SCENE_SCENE_PARTICLES_H_
#define SPLASH_SCENE_SCENE_PARTICLES_H_

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

namespace scene
{
class Resources;

class SceneParticles final : public Scene
{
public:
  SceneParticles() = delete;
  SceneParticles(Resources* resources, gl::Shaders* shaders);
  ~SceneParticles() override;

  void drawUi() override;
  void draw() override;

private:
  Resources* resources_ = nullptr;
  gl::Shaders* shaders_ = nullptr;

  void initializeParticles();
  void updateParticles(float dt);

  std::unique_ptr<geom::Particles> particles_;
  std::unique_ptr<gl::ParticlesGeometry> particlesGeometry_;

  // Simulation
  std::vector<glm::vec3> prevPositions_;

  // UI
  std::chrono::high_resolution_clock::time_point lastTime_;

  uint32_t particleCountX_ = 32;
  uint32_t particleCountY_ = 32;
  bool animation_ = false;
};
}
}

#endif // SPLASH_SCENE_SCENE_PARTICLES_H_
