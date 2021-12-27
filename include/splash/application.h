#ifndef SPLASH_APPLICATION_H_
#define SPLASH_APPLICATION_H_

#include <cstdint>

struct GLFWwindow;

namespace splash
{
class Application
{
public:
  Application();
  ~Application();

  void run();

private:
  GLFWwindow* window_ = nullptr;

  uint32_t width_ = 1600;
  uint32_t height_ = 900;
};
}

#endif // SPLASH_APPLICATION_H_
