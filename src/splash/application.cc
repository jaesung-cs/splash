#include <splash/application.h>

#include <chrono>
#include <thread>
#include <iostream>
#include <chrono>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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
}

Application::Application()
{
  if (!glfwInit())
    throw std::runtime_error("Failed to initialize glfw");

  glfwSetErrorCallback(errorCallback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_SAMPLES, 4);

  width_ = 1600;
  height_ = 900;
  window_ = glfwCreateWindow(width_, height_, "Splash", NULL, NULL);
  if (!window_)
    throw std::runtime_error("Failed to create window");

  glfwSetWindowPos(window_, 300, 100);

  // Callbacks
  glfwSetWindowUserPointer(window_, this);

  glfwMakeContextCurrent(window_);

  if (!gladLoadGL())
    throw std::runtime_error("Failed to initialize GL");

  glEnable(GL_MULTISAMPLE);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  const char* glslVersion = "#version 430";
  ImGui_ImplGlfw_InitForOpenGL(window_, true);
  ImGui_ImplOpenGL3_Init(glslVersion);
}

Application::~Application()
{
  // ImGui Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  floorShader_ = nullptr;
  floorTexture_ = nullptr;
  floorGeometry_ = nullptr;

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

    handleEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Begin("Control"))
    {
      if (ImGui::Button("Hello World"))
        std::cout << "Hello World!" << std::endl;
    }
    ImGui::End();

    ImGui::Render();

    draw();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window_);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(0.001s);
  }
}

void Application::handleEvents()
{
  const auto& io = ImGui::GetIO();

  // Resize
  width_ = io.DisplaySize[0];
  height_ = io.DisplaySize[1];
  camera_->setAspect(static_cast<float>(width_) / height_);

  // Mouse
  if (!io.WantCaptureMouse && io.MousePos.x != -FLT_MAX && io.MousePos.y != -FLT_MAX)
  {
    auto dx = static_cast<int>(io.MouseDelta.x);
    auto dy = static_cast<int>(io.MouseDelta.y);

    if (io.MouseDown[0] && !io.MouseDown[1])
      camera_->rotateByPixels(dx, dy);
    else if (!io.MouseDown[0] && io.MouseDown[1])
      camera_->translateByPixels(dx, dy);
    else if (io.MouseDown[0] && io.MouseDown[1])
      camera_->zoomByPixels(dx, dy);
  }

  // Scroll
  camera_->zoomByScroll(io.MouseWheelH, io.MouseWheel);

  // Keyboard
  if (!io.WantCaptureKeyboard)
  {
    const auto dt = io.DeltaTime;
    if (io.KeysDown['W']) camera_->moveForward(dt * cameraSpeed_ * camera_->distance());
    if (io.KeysDown['S']) camera_->moveForward(-dt * cameraSpeed_ * camera_->distance());
    if (io.KeysDown['A']) camera_->moveRight(-dt * cameraSpeed_ * camera_->distance());
    if (io.KeysDown['D']) camera_->moveRight(dt * cameraSpeed_ * camera_->distance());
    if (io.KeysDown[' ']) camera_->moveUp(dt * cameraSpeed_ * camera_->distance());
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
