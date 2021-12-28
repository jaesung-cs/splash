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
#include <splash/geom/particles.h>
#include <splash/gl/shader.h>
#include <splash/gl/texture.h>
#include <splash/gl/geometry.h>
#include <splash/gl/particles_geometry.h>

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

  // Initialize OpenGL configurations
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);

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
  particlesShader_ = nullptr;

  floorTexture_ = nullptr;
  floorGeometry_ = nullptr;
  particlesGeometry_ = nullptr;

  glfwTerminate();
}

void Application::run()
{
  // Shaders
  const std::string baseDirectory = "C:\\workspace\\splash\\src\\splash\\shader";
  floorShader_ = std::make_unique<gl::Shader>(baseDirectory, "floor");
  particlesShader_ = std::make_unique<gl::Shader>(baseDirectory, "particles");

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

  // Particles
  particles_ = std::make_unique<geom::Particles>(particleCount_);
  particlesGeometry_ = std::make_unique<gl::ParticlesGeometry>(particleCount_);
  
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

    updateParticles(0.f);

    updateLights();

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

void Application::updateParticles(float animationTime)
{
  auto& particles = *particles_;

  for (int i = 0; i < particleCount_; i++)
  {
    const auto t = static_cast<float>(i) / (particleCount_ - 1);
    const auto x = static_cast<float>(i);

    particles[i].position = { x, 0.f, 0.f };
    particles[i].radius = t + 0.1f;
    particles[i].color = { 0.f, 0.f, t };
  }

  particlesGeometry_->update(particles);
}

void Application::updateLights()
{
  lights_.resize(2);

  // Eye camera
  lights_[0].position = { camera_->eye(), 1.f };
  lights_[0].ambient = { 0.1f, 0.1f, 0.1f, 1.f };
  lights_[0].diffuse = { 0.5f, 0.5f, 0.5f, 0.5f };
  lights_[0].specular = { 0.1f, 0.1f, 0.1f, 1.f };

  // Directional camera
  lights_[1].position = { 0.f, 0.f, 1.f, 0.f };
  lights_[1].ambient = { 0.1f, 0.1f, 0.1f, 1.f };
  lights_[1].diffuse = { 0.5f, 0.5f, 0.5f, 1.f };
  lights_[1].specular = { 0.1f, 0.1f, 0.1f, 1.f };
}

void Application::draw()
{
  glClearColor(0.75f, 0.75f, 0.75f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, width_, height_);

  // Draw floor
  {
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

    floorShader_->done();
  }

  // Draw particles
  {
    particlesShader_->use();

    glm::mat4 model = glm::mat4(1.f);
    glm::mat4 modelInverseTranspose = glm::transpose(glm::inverse(model));
    glm::mat4 view = camera_->view();
    glm::mat4 projection = camera_->projection();

    particlesShader_->uniformMatrix4f("model", model);
    particlesShader_->uniformMatrix4f("modelInverseTranspose", modelInverseTranspose);
    particlesShader_->uniformMatrix4f("view", view);
    particlesShader_->uniformMatrix4f("projection", projection);

    constexpr float shininess = 16.f;
    particlesShader_->uniform3f("eye", camera_->eye());
    particlesShader_->uniform1f("shininess", shininess);
    particlesShader_->uniform1i("numLights", lights_.size());
    for (int i = 0; i < lights_.size(); i++)
    {
      const auto& light = lights_[i];
      const std::string base = "lights[" + std::to_string(i) + "]";
      particlesShader_->uniform4f(base + ".position", light.position);
      particlesShader_->uniform4f(base + ".ambient", light.ambient);
      particlesShader_->uniform4f(base + ".diffuse", light.diffuse);
      particlesShader_->uniform4f(base + ".specular", light.specular);
    }

    particlesGeometry_->draw();

    particlesShader_->done();
  }
}
}
