#include <splash/scene/scene_fluid.h>

#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <splash/gl/shaders.h>
#include <splash/gl/shader.h>
#include <splash/gl/texture.h>
#include <splash/gl/geometry.h>
#include <splash/gl/particles_geometry.h>
#include <splash/geom/particles.h>
#include <splash/model/camera.h>
#include <splash/scene/resources.h>

namespace splash
{
namespace scene
{
SceneFluid::SceneFluid(Resources* resources, gl::Shaders* shaders)
  : Scene()
  , resources_(resources)
  , shaders_(shaders)
{
  particles_ = std::make_unique<geom::Particles>(particleCount_);
  particlesGeometry_ = std::make_unique<gl::ParticlesGeometry>(particleCount_);

  lastTime_ = std::chrono::high_resolution_clock::now();

  initializeParticles();
}

SceneFluid::~SceneFluid() = default;

void SceneFluid::drawUi()
{
  if (ImGui::Button("Initialize"))
    initializeParticles();

  ImGui::Checkbox("Animation", &animation_);
}

void SceneFluid::draw()
{
  const auto now = std::chrono::high_resolution_clock::now();
  const auto dt = std::chrono::duration<float>(now - lastTime_).count();
  lastTime_ = now;

  if (animation_)
    animationTime_ += dt;

  updateParticles(dt);

  auto& camera = resources_->camera();

  // Draw floor
  {
    auto& floorShader = (*shaders_)["floor"];
    floorShader.use();

    glm::mat4 model = glm::mat4(1.f);
    glm::mat4 modelInverseTranspose = glm::transpose(glm::inverse(model));
    glm::mat4 view = camera.view();
    glm::mat4 projection = camera.projection();

    auto& floorTexture = resources_->floorTexture();
    auto& floorGeometry = resources_->floorGeometry();

    floorShader.uniformMatrix4f("model", model);
    floorShader.uniformMatrix4f("modelInverseTranspose", modelInverseTranspose);
    floorShader.uniformMatrix4f("view", view);
    floorShader.uniformMatrix4f("projection", projection);
    floorShader.uniform1i("tex", 0);
    floorTexture.bind(0);
    floorGeometry.draw();

    floorShader.done();
  }

  // Draw particles
  {
    auto& particlesShader = (*shaders_)["particles"];
    particlesShader.use();

    glm::mat4 model = glm::mat4(1.f);
    glm::mat4 modelInverseTranspose = glm::transpose(glm::inverse(model));
    glm::mat4 view = camera.view();
    glm::mat4 projection = camera.projection();

    const auto& lights = resources_->lights();

    particlesShader.uniformMatrix4f("model", model);
    particlesShader.uniformMatrix4f("modelInverseTranspose", modelInverseTranspose);
    particlesShader.uniformMatrix4f("view", view);
    particlesShader.uniformMatrix4f("projection", projection);

    particlesShader.uniform1f("radius", particles_->radius());

    constexpr float shininess = 16.f;
    particlesShader.uniform3f("eye", camera.eye());
    particlesShader.uniform1f("shininess", shininess);
    particlesShader.uniform1i("numLights", lights.size());
    for (int i = 0; i < lights.size(); i++)
    {
      const auto& light = lights[i];
      const std::string base = "lights[" + std::to_string(i) + "]";
      particlesShader.uniform4f(base + ".position", light.position);
      particlesShader.uniform4f(base + ".ambient", light.ambient);
      particlesShader.uniform4f(base + ".diffuse", light.diffuse);
      particlesShader.uniform4f(base + ".specular", light.specular);
    }

    particlesGeometry_->draw();

    particlesShader.done();
  }
}

void SceneFluid::initializeParticles()
{
  auto& particles = *particles_;
  constexpr float radius = 1.f / particleSide_;

  particles.radius() = radius;

  constexpr float pi = 3.1415926535897932384626433832795f;
  constexpr float density = 997.f;
  const auto volume = 4.f / 3.f * pi * radius * radius * radius;
  const auto mass = density * volume;

  constexpr float baseHeight = 1.f;

  for (int i = 0; i < particleSide_; i++)
  {
    const auto u = static_cast<float>(i) / (particleSide_ - 1);
    for (int j = 0; j < particleSide_; j++)
    {
      const auto v = static_cast<float>(j) / (particleSide_ - 1);
      for (int k = 0; k < particleSide_; k++)
      {
        const auto w = static_cast<float>(k) / (particleSide_ - 1);

        const auto index = i * particleSide_ * particleSide_ + j * particleSide_ + k;

        auto& particle = particles[index];
        particle.position = { u, v, w + baseHeight };
        particle.mass = mass;
        particle.velocity = { 0.f, 0.f, 0.f };
        particle.color = { 0.f, 0.f, 1.f };
      }
    }
  }
}

void SceneFluid::updateParticles(float dt)
{
  auto& particles = *particles_;
  const auto radius = particles_->radius();

  if (animation_)
  {
    constexpr glm::vec3 gravity = { 0.f, 0.f, -9.80665f };

    for (int i = 0; i < particleCount_; i++)
    {
      particles[i].velocity += gravity * dt;
      particles[i].position += particles[i].velocity * dt;
      particles[i].position.z = std::max(particles[i].position.z, 0.f);
    }
  }

  particlesGeometry_->update(*particles_);
}
}
}
