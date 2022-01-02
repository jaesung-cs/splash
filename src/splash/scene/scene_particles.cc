#include <splash/scene/scene_particles.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>

#include <splash/scene/resources.h>
#include <splash/gl/shaders.h>
#include <splash/gl/shader.h>
#include <splash/gl/texture.h>
#include <splash/gl/geometry.h>
#include <splash/gl/particles_geometry.h>
#include <splash/geom/particles.h>
#include <splash/model/camera.h>

namespace splash
{
namespace scene
{
SceneParticles::SceneParticles(Resources* resources, gl::Shaders* shaders)
  : Scene()
  , resources_(resources)
  , shaders_(shaders)
{
  initializeParticles();
}

SceneParticles::~SceneParticles() = default;

void SceneParticles::initializeParticles()
{
  const auto particleCount = particleCountX_ * particleCountY_;

  particles_ = std::make_unique<geom::Particles>(particleCount);
  particlesGeometry_ = std::make_unique<gl::ParticlesGeometry>(particleCount);

  constexpr float baseHeight = 1.f;
  constexpr float radius = 0.005f; // 0.5cm
  constexpr float density = 0.175;
  constexpr float mass = 8.f * radius * radius * radius; // Cubic particle

  particles_->radius() = radius;
  for (int i = 0; i < particleCountX_; i++)
  {
    const auto u = static_cast<float>(i) / particleCountX_;
    for (int j = 0; j < particleCountY_; j++)
    {
      const auto v = static_cast<float>(j) / particleCountY_;

      const auto index = i * particleCountY_ + j;
      auto& particle = (*particles_)[index];
      particle.position = { i * radius * 2.f, 0, baseHeight - j * radius * 2.f};
      particle.velocity = { 0.f, 0.f, 0.f };
      particle.mass = mass;
      particle.color = { 0.5f, 0.5f, v };
    }
  }

  particlesGeometry_->update(*particles_);
}

void SceneParticles::drawUi()
{
  int slider[2] = {
    particleCountX_,
    particleCountY_,
  };
  if (ImGui::SliderInt2("Particle Count", slider, 16, 64))
  {
    particleCountX_ = slider[0];
    particleCountY_ = slider[1];
  }

  if (ImGui::Button("Initialize"))
    initializeParticles();
}

void SceneParticles::draw()
{
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

  // Particles
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
}
}
