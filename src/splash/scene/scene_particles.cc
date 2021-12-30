#include <splash/scene/scene_particles.h>

#include <glad/glad.h>

#include <splash/model/camera.h>
#include <splash/scene/resources.h>
#include <splash/geom/particles.h>
#include <splash/gl/shader.h>
#include <splash/gl/shaders.h>
#include <splash/gl/texture.h>
#include <splash/gl/geometry.h>
#include <splash/gl/particles_geometry.h>

namespace splash
{
namespace scene
{
SceneParticles::SceneParticles(Resources* resources, gl::Shaders* shaders)
  : Scene()
  , resources_(resources)
  , shaders_(shaders)
{
  // Particles
  particles_ = std::make_unique<geom::Particles>(particleCount_);
  particlesGeometry_ = std::make_unique<gl::ParticlesGeometry>(particleCount_);
}

SceneParticles::~SceneParticles() = default;

std::string SceneParticles::name() const
{
  return "Particles";
}

void SceneParticles::drawUi()
{
}

void SceneParticles::draw()
{
  updateParticles(0.f);

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

void SceneParticles::updateParticles(float animationTime)
{
  auto& particles = *particles_;

  for (int i = 0; i < particleCount_; i++)
  {
    const auto t = static_cast<float>(i) / (particleCount_ - 1);
    const auto x = static_cast<float>(i);

    constexpr float speed = 10.f;
    const auto cosT = std::cos(animationTime * speed * t);
    const auto sinT = std::sin(animationTime * speed * t);

    particles[i].position = { t * cosT, t * sinT, t };
    particles[i].radius = 0.1f * t;
    particles[i].color = { 0.f, 0.f, t };
  }

  particlesGeometry_->update(particles);
}

}
}
