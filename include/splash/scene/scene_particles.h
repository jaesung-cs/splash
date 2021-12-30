#ifndef SPLASH_SCENE_SCENE_PARTICLES_H_
#define SPLASH_SCENE_SCENE_PARTICLES_H_

#include <splash/scene/scene.h>

#include <chrono>
#include <memory>

namespace splash
{
namespace geom
{
class Particles;
}

namespace gl
{
class Shaders;
class ParticlesGeometry;
class BoxesGeometry;
}

namespace scene
{
class Resources;

class SceneParticles final : public Scene
{
public:
  SceneParticles() = delete;
  explicit SceneParticles(Resources* resources, gl::Shaders* shaders);
  ~SceneParticles() override;

  std::string name() const override;

  void drawUi() override;
  void draw() override;

private:
  Resources* resources_ = nullptr;
  gl::Shaders* shaders_ = nullptr;

  void updateParticles(float animationTime);

  static constexpr uint32_t particleCount_ = 128;
  std::unique_ptr<geom::Particles> particles_;
  std::unique_ptr<gl::ParticlesGeometry> particlesGeometry_;

  std::unique_ptr<gl::BoxesGeometry> boxesGeometry_;

  // Animation
  float animationTime_ = 0.f;
  std::chrono::high_resolution_clock::time_point lastTime_;

  // Rendering options
  bool drawBoxes_ = false;
  bool animation_ = true;
};
}
}

#endif // SPLASH_SCENE_SCENE_PARTICLES_H_
