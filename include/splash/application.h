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

  // Callbacks
  void resize(uint32_t width, uint32_t height);
  void cursor(float x, float y);
  void mouse(int button, int action);
  void scroll(float dx, float dy);
  void keyboard(int key, int action);

private:
  void draw();

  GLFWwindow* window_ = nullptr;

  uint32_t width_ = 1600;
  uint32_t height_ = 900;

  std::unique_ptr<model::Camera> camera_;

  std::unique_ptr<gl::Shader> floorShader_;
  std::unique_ptr<gl::Texture> floorTexture_;
  std::unique_ptr<gl::Geometry> floorGeometry_;

  // UI
  glm::vec2 lastCursorPos_{ 0.f, 0.f };
  std::array<bool, 2> mouseButtons_{ false, false };
  std::array<bool, 256> keys_{ false };
};
}

#endif // SPLASH_APPLICATION_H_
