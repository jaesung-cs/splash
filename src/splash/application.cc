#include <splash/application.h>

#include <chrono>
#include <thread>
#include <iostream>
#include <chrono>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <splash/model/camera.h>
#include <splash/model/image.h>
#include <splash/gl/shader.h>
#include <splash/gl/texture.h>
#include <splash/gl/geometry.h>

namespace splash
{
namespace
{
void errorCallback(int error, const char* description)
{
  std::cerr << "Error: " << description << std::endl;
}

void resizeCallback(GLFWwindow* window, int width, int height)
{
  auto* application = static_cast<Application*>(glfwGetWindowUserPointer(window));
  application->resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}

void cursorCallback(GLFWwindow* window, double xpos, double ypos)
{
  auto* application = static_cast<Application*>(glfwGetWindowUserPointer(window));
  application->cursor(static_cast<float>(xpos), static_cast<float>(ypos));
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
  auto* application = static_cast<Application*>(glfwGetWindowUserPointer(window));
  application->mouse(button, action);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  auto* application = static_cast<Application*>(glfwGetWindowUserPointer(window));
  application->scroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  auto* application = static_cast<Application*>(glfwGetWindowUserPointer(window));
  application->keyboard(key, action);
}
}

Application::Application()
{
  if (!glfwInit())
    throw std::runtime_error("Failed to initialize glfw");

  glfwSetErrorCallback(errorCallback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  width_ = 1600;
  height_ = 900;
  window_ = glfwCreateWindow(width_, height_, "Splash", NULL, NULL);
  if (!window_)
    throw std::runtime_error("Failed to create window");

  glfwSetWindowPos(window_, 300, 100);

  // Callbacks
  glfwSetWindowUserPointer(window_, this);
  glfwSetWindowSizeCallback(window_, resizeCallback);
  glfwSetCursorPosCallback(window_, cursorCallback);
  glfwSetMouseButtonCallback(window_, mouseCallback);
  glfwSetScrollCallback(window_, scrollCallback);
  glfwSetKeyCallback(window_, keyboardCallback);

  glfwMakeContextCurrent(window_);

  if (!gladLoadGL())
    throw std::runtime_error("Failed to initialize GL");

  glEnable(GL_MULTISAMPLE);
}

Application::~Application()
{
  floorShader_ = nullptr;
  floorTexture_ = nullptr;
  floorGeometry_ = nullptr;

  glfwTerminate();
}

void Application::resize(uint32_t width, uint32_t height)
{
  width_ = width;
  height_ = height;
}

void Application::cursor(float x, float y)
{
  glm::vec2 delta = glm::vec2(x, y) - lastCursorPos_;
  auto dx = static_cast<int>(delta.x);
  auto dy = static_cast<int>(delta.y);

  if (mouseButtons_[0] && !mouseButtons_[1])
    camera_->rotateByPixels(dx, dy);
  else if (!mouseButtons_[0] && mouseButtons_[1])
    camera_->translateByPixels(dx, dy);
  else if (mouseButtons_[0] && mouseButtons_[1])
    camera_->zoomByPixels(dx, dy);

  lastCursorPos_ = { x, y };
}

void Application::mouse(int button, int action)
{
  switch (button)
  {
  case GLFW_MOUSE_BUTTON_LEFT: button = 0; break;
  case GLFW_MOUSE_BUTTON_RIGHT: button = 1; break;
  default: button = 0; break;
  }

  bool pressed = false;
  switch (action)
  {
  case GLFW_PRESS: pressed = true; break;
  case GLFW_RELEASE: pressed = false; break;
  }

  mouseButtons_[button] = pressed;
}

void Application::scroll(float dx, float dy)
{
  camera_->zoomByScroll(dx, dy);
}

void Application::keyboard(int key, int action)
{
  if (key < 256)
  {
    bool pressed = false;
    switch (action)
    {
    case GLFW_PRESS:
    case GLFW_REPEAT:
      pressed = true;
      break;

    case GLFW_RELEASE:
      pressed = false;
      break;
    }

    keys_[key] = pressed;
  }
}

void Application::run()
{
  // Floor texture
  constexpr uint32_t floorPixelSize = 256;
  constexpr uint32_t border = 2;
  model::Image floorImage(floorPixelSize, floorPixelSize, 3);
  for (int i = 0; i < floorPixelSize; i++)
  {
    for (int j = 0; j < floorPixelSize; j++)
    {
      uint32_t color = 192;
      if (i < border || i >= floorPixelSize - border ||
        j < border || j >= floorPixelSize - border)
        color = 16;

      for (int c = 0; c < 3; c++)
        floorImage(i, j, c) = color;
    }
  }

  floorTexture_ = std::make_unique<gl::Texture>(floorImage);

  // Floor shader
  floorShader_ = std::make_unique<gl::Shader>("C:\\workspace\\splash\\src\\splash\\shader", "floor");

  // Floor geometry
  {
    constexpr float floorLength = 10.f;
    std::vector<float> vertex{
      -floorLength, -floorLength, 0.f, 0.f, 0.f, 1.f, -floorLength, -floorLength,
      floorLength, -floorLength, 0.f, 0.f, 0.f, 1.f, floorLength, -floorLength,
      -floorLength, floorLength, 0.f, 0.f, 0.f, 1.f, -floorLength, floorLength,
      floorLength, floorLength, 0.f, 0.f, 0.f, 1.f, floorLength, floorLength,
    };

    std::vector<uint32_t> index{
      0, 1, 2, 2, 1, 3,
    };

    floorGeometry_ = std::make_unique<gl::Geometry>(
      vertex, index,
      std::initializer_list<gl::Attribute>{
        { 0, 3, 8, 0 },
        { 1, 3, 8, 3 },
        { 2, 2, 8, 6 },
      }
    );
  }

  // Camera
  camera_ = std::make_unique<model::Camera>(60.f / 180.f * glm::pi<float>(), static_cast<float>(width_) / height_);

  auto lastTimestamp = std::chrono::high_resolution_clock::now();
  while (!glfwWindowShouldClose(window_))
  {
    auto now = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration<float>(now - lastTimestamp).count();
    lastTimestamp = now;

    glfwPollEvents();

    // Keyboard movement
    if (keys_['W']) camera_->moveForward(dt * cameraSpeed_ * camera_->distance());
    if (keys_['S']) camera_->moveForward(-dt * cameraSpeed_ * camera_->distance());
    if (keys_['A']) camera_->moveRight(-dt * cameraSpeed_ * camera_->distance());
    if (keys_['D']) camera_->moveRight(dt * cameraSpeed_ * camera_->distance());
    if (keys_[' ']) camera_->moveUp(dt * cameraSpeed_ * camera_->distance());

    draw();

    glfwSwapBuffers(window_);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(0.001s);
  }
}

void Application::draw()
{
  glClearColor(0.75f, 0.75f, 0.75f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, width_, height_);

  floorShader_->use();

  glm::mat4 model = glm::mat4(1.f);
  glm::mat4 modelInverseTranspose = glm::transpose(glm::inverse(model));
  glm::mat4 view = camera_->view();
  glm::mat4 projection = camera_->projection();

  floorShader_->uniformMatrix4f("model", model);
  floorShader_->uniformMatrix4f("modelInverseTranspose", modelInverseTranspose);
  floorShader_->uniformMatrix4f("view", view);
  floorShader_->uniformMatrix4f("projection", projection);
  floorShader_->uniform1i("tex", 0);
  floorTexture_->bind(0);
  floorGeometry_->draw();
}
}
