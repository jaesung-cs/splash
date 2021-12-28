#ifndef SPLASH_APPLICATION_H_
#define SPLASH_APPLICATION_H_

#include <cstdint>
#include <memory>
#include <array>

#include <glm/glm.hpp>

struct GLFWwindow;

namespace splash
{
namespace gl
{
class Shader;
class Texture;
class Geometry;
class ParticlesGeometry;
}

namespace geom
{
class Particles;
}

namespace model
{
class Camera;
}

class Application
{
public:
  Application();
  ~Application();

  void run();

private:
  void handleEvents();
  void updateParticles(float animationTime);
  void draw();

  GLFWwindow* window_ = nullptr;

  uint32_t width_ = 1600;
  uint32_t height_ = 900;

  std::unique_ptr<model::Camera> camera_;

  std::unique_ptr<gl::Shader> floorShader_;
  std::unique_ptr<gl::Shader> particlesShader_;

  std::unique_ptr<gl::Texture> floorTexture_;
  std::unique_ptr<gl::Geometry> floorGeometry_;

  static constexpr uint32_t particleCount_ = 16;
  std::unique_ptr<geom::Particles> particles_;
  std::unique_ptr<gl::ParticlesGeometry> particlesGeometry_;

  // UI
  float cameraSpeed_ = 3.f;
};
}

#endif // SPLASH_APPLICATION_H_
