#include <splash/application.h>

#include <chrono>
#include <thread>
#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

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
}

Application::Application()
{
  if (!glfwInit())
    throw std::runtime_error("Failed to initialize glfw");

  glfwSetErrorCallback(errorCallback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  window_ = glfwCreateWindow(width_, height_, "Splash", NULL, NULL);
  if (!window_)
    throw std::runtime_error("Failed to create window");

  // Callbacks
  glfwSetWindowUserPointer(window_, this);

  glfwMakeContextCurrent(window_);

  if (!gladLoadGL())
    throw std::runtime_error("Failed to initialize GL");
}

Application::~Application()
{
  glfwTerminate();
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
    constexpr float floorLength = 100.f;
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

  while (!glfwWindowShouldClose(window_))
  {
    glfwPollEvents();

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

  // TODO
  glm::mat4 view = glm::mat4(1.f);
  glm::mat4 projection = glm::mat4(1.f);

  floorShader_->uniformMatrix4f("model", model);
  floorShader_->uniformMatrix4f("modelInverseTranspose", modelInverseTranspose);
  floorShader_->uniformMatrix4f("view", view);
  floorShader_->uniformMatrix4f("projection", projection);
  floorShader_->uniform1i("tex", 0);
  floorTexture_->bind(0);
  floorGeometry_->draw();
}
}