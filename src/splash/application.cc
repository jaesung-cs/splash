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
#include <splash/gl/shaders.h>
#include <splash/gl/texture.h>
#include <splash/gl/geometry.h>
#include <splash/gl/particles_geometry.h>
#include <splash/scene/resources.h>
#include <splash/scene/scene_animation.h>
#include <splash/scene/scene_particles.h>

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

  shaders_ = nullptr;
  resources_ = nullptr;

  scenes_.clear();

  glfwTerminate();
}

void Application::initializeScenes()
{
  scenes_.push_back(std::make_unique<scene::SceneParticles>(*resources_, *shaders_));
  scenes_.push_back(std::make_unique<scene::SceneAnimation>());
}

void Application::run()
{
  // Shaders
  shaders_ = std::make_unique<gl::Shaders>();

  // Resources
  resources_ = std::make_unique<scene::Resources>();

  // Initialize scenes
  initializeScenes();

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
      const auto currentSceneName = scenes_[sceneIndex_]->name();
      if (ImGui::BeginCombo("Scene", currentSceneName.c_str()))
      {
        for (int i = 0; i < scenes_.size(); i++)
        {
          const auto name = scenes_[i]->name();
          bool isSelected = false;
          if (ImGui::Selectable(name.c_str(), &isSelected))
            sceneIndex_ = i;

          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
      }

      ImGui::Separator();

      // Draw per-scene UI
      if (ImGui::CollapsingHeader("Scene Control", ImGuiTreeNodeFlags_DefaultOpen))
      {
        scenes_[sceneIndex_]->drawUi();
      }

      ImGui::Separator();
    }
    ImGui::End();

    ImGui::Render();

    updateLights();

    glClearColor(0.75f, 0.75f, 0.75f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width_, height_);

    scenes_[sceneIndex_]->draw();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window_);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(0.001s);
  }
}

void Application::handleEvents()
{
  const auto& io = ImGui::GetIO();

  auto& camera = resources_->camera();

  // Resize
  width_ = io.DisplaySize[0];
  height_ = io.DisplaySize[1];
  camera.setAspect(static_cast<float>(width_) / height_);

  // Mouse
  if (!io.WantCaptureMouse && io.MousePos.x != -FLT_MAX && io.MousePos.y != -FLT_MAX)
  {
    auto dx = static_cast<int>(io.MouseDelta.x);
    auto dy = static_cast<int>(io.MouseDelta.y);

    if (io.MouseDown[0] && !io.MouseDown[1])
      camera.rotateByPixels(dx, dy);
    else if (!io.MouseDown[0] && io.MouseDown[1])
      camera.translateByPixels(dx, dy);
    else if (io.MouseDown[0] && io.MouseDown[1])
      camera.zoomByPixels(dx, dy);
  }

  // Scroll
  camera.zoomByScroll(io.MouseWheelH, io.MouseWheel);

  // Keyboard
  if (!io.WantCaptureKeyboard)
  {
    const auto dt = io.DeltaTime;
    if (io.KeysDown['W']) camera.moveForward(dt * cameraSpeed_ * camera.distance());
    if (io.KeysDown['S']) camera.moveForward(-dt * cameraSpeed_ * camera.distance());
    if (io.KeysDown['A']) camera.moveRight(-dt * cameraSpeed_ * camera.distance());
    if (io.KeysDown['D']) camera.moveRight(dt * cameraSpeed_ * camera.distance());
    if (io.KeysDown[' ']) camera.moveUp(dt * cameraSpeed_ * camera.distance());
  }
}

void Application::updateLights()
{
  auto& camera = resources_->camera();
  auto& lights = resources_->lights();

  lights.resize(2);

  // Eye camera
  lights[0].position = { camera.eye(), 1.f };
  lights[0].ambient = { 0.1f, 0.1f, 0.1f, 1.f };
  lights[0].diffuse = { 0.5f, 0.5f, 0.5f, 0.5f };
  lights[0].specular = { 0.1f, 0.1f, 0.1f, 1.f };

  // Directional camera
  lights[1].position = { 0.f, 0.f, 1.f, 0.f };
  lights[1].ambient = { 0.1f, 0.1f, 0.1f, 1.f };
  lights[1].diffuse = { 0.5f, 0.5f, 0.5f, 1.f };
  lights[1].specular = { 0.1f, 0.1f, 0.1f, 1.f };
}
}
