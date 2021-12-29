#ifndef SPLASH_SCENE_SCENE_PARTICLES_H_
#define SPLASH_SCENE_SCENE_PARTICLES_H_

#include <splash/scene/scene.h>

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
}

namespace scene
{
class Resources;

class SceneParticles final : public Scene
{
public:
  SceneParticles() = delete;
  explicit SceneParticles(Resources& resources, gl::Shaders& shaders);
  ~SceneParticles() override;

  std::string name() const override;

  void drawUi() override;
  void draw() override;

private:
  Resources& resources_;
  gl::Shaders& shaders_;

  void updateParticles(float animationTime);

  static constexpr uint32_t particleCount_ = 128;
  std::unique_ptr<geom::Particles> particles_;
  std::unique_ptr<gl::ParticlesGeometry> particlesGeometry_;
};
}
}

#endif // SPLASH_SCENE_SCENE_PARTICLES_H_