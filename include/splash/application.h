#ifndef SPLASH_APPLICATION_H_
#define SPLASH_APPLICATION_H_

#include <cstdint>
#include <memory>

struct GLFWwindow;

namespace splash
{
namespace gl
{
class Shader;
class Texture;
class Geometry;
}

class Application
{
public:
  Application();
  ~Application();

  void run();

private:
  void draw();

  GLFWwindow* window_ = nullptr;

  uint32_t width_ = 1600;
  uint32_t height_ = 900;

  std::unique_ptr<gl::Shader> floorShader_;
  std::unique_ptr<gl::Texture> floorTexture_;
  std::unique_ptr<gl::Geometry> floorGeometry_;
};
}

#endif // SPLASH_APPLICATION_H_
