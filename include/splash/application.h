#ifndef SPLASH_APPLICATION_H_
#define SPLASH_APPLICATION_H_

#include <cstdint>
#include <memory>
#include <array>
#include <vector>

#include <glm/glm.hpp>

struct GLFWwindow;

namespace splash
{
namespace gl
{
class Shaders;
class Texture;
class Geometry;
class ParticlesGeometry;
}

namespace geom
{
class Particles;
}

namespace scene
{
class Scene;
class Resources;
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
  void initializeScenes();

  void handleEvents();
  void updateLights();

  GLFWwindow* window_ = nullptr;

  uint32_t width_ = 1600;
  uint32_t height_ = 900;

  std::unique_ptr<gl::Shaders> shaders_;
  std::unique_ptr<scene::Resources> resources_;

  // UI
  float cameraSpeed_ = 3.f;

  // Scenes
  int sceneIndex_ = 0;
  std::vector<std::unique_ptr<scene::Scene>> scenes_;
};
}

#endif // SPLASH_APPLICATION_H_
