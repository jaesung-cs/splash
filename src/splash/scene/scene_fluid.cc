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
#include <splash/fluid/neighbor_search_spatial_hashing.h>

namespace splash
{
namespace scene
{
namespace
{
constexpr float pi = 3.1415926535897932384626433832795f;

float Poly6(const glm::vec3& r, float h)
{
  const auto h2 = h * h;
  const auto r2 = glm::dot(r, r);
  if (r2 > h2)
    return 0.f;

  const auto h3 = h2 * h;
  const auto f = (h2 - r2) / h3;
  const auto f3 = f * f * f;
  return 315.f / 64.f / pi * f3;
}

glm::vec3 gradPoly6(const glm::vec3& r, float h)
{
  const auto h2 = h * h;
  const auto r2 = glm::dot(r, r);
  if (r2 > h2)
    return glm::vec3(0.f);

  const auto h4 = h2 * h2;
  const auto f = (h2 - r2) / h4;
  const auto f2 = f * f;
  return -945.f / 32.f / pi * f2 * (r / h);
}
}

SceneFluid::SceneFluid(Resources* resources, gl::Shaders* shaders)
  : Scene()
  , resources_(resources)
  , shaders_(shaders)
{
  particles_ = std::make_unique<geom::Particles>(particleCount_);
  particlesGeometry_ = std::make_unique<gl::ParticlesGeometry>(particleCount_);

  lastTime_ = std::chrono::high_resolution_clock::now();

  neighborSearch_ = std::make_unique<fluid::NeighborSearchSpatialHashing>();

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
  auto dt = std::chrono::duration<float>(now - lastTime_).count();
  lastTime_ = now;

  // Slow motion
  dt /= 10.f;

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
  constexpr float radiusFactor = 1.4f;
  constexpr float radius = 1.f / particleSide_ / radiusFactor;

  particles.radius() = radius;

  rho0_ = 997.f;

  constexpr float pi = 3.1415926535897932384626433832795f;
  const auto volume = 4.f / 3.f * pi * radius * radius * radius; // Spherical particle
  const auto mass = rho0_ * volume;

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
        particle.position = { u + w * 0.1f, v + w * 0.1f, w + baseHeight };
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
    const auto n = static_cast<uint32_t>(particles.size());

    constexpr glm::vec3 gravity = { 0.f, 0.f, -9.80665f };
    positions_.resize(n);
    for (int i = 0; i < n; i++)
    {
      // Store old particle positions
      positions_[i] = particles[i].position;

      // Update particles
      particles[i].velocity += gravity * dt;
      particles[i].position += particles[i].velocity * dt;

      // Plane constraints
      if (particles[i].position.z < 0.f)
        particles[i].position.z = -0.75f * particles[i].position.z;
    }

    // Neighbor search
    const auto h = 4.f * radius; // SPH support radius
    neighborSearch_->computeNeighbors(particles, h);
    const auto& neighbors = neighborSearch_->neighbors();

    // TODO: Move fluid simulation to a class
    // Density calculation
    density_.resize(n);
    for (int i = 0; i < n; i++)
      density_[i] = particles[i].mass * Poly6(glm::vec3(0.f), h);

    for (const auto& neighbor : neighbors)
    {
      const auto i0 = neighbor.i0;
      const auto i1 = neighbor.i1;

      const auto& p0 = particles[i0].position;
      const auto& p1 = particles[i1].position;

      // The opposite direction is already in neighbor list
      density_[i0] += particles[i1].mass * Poly6(p0 - p1, h);
    }

    // Compute density constraints
    incompressibility_.resize(n);
    for (int i = 0; i < n; i++)
      incompressibility_[i] = std::max(density_[i] / rho0_ - 1.f, 0.f);

    // Project to make incompressibility = 0
    incompressibilitySelfGrad_.resize(n);
    incompressibilityDenoms_.resize(n);
    for (int i = 0; i < n; i++)
    {
      incompressibilitySelfGrad_[i] = glm::vec3(0.f);
      incompressibilityDenoms_[i] = 0.f;
    }

    for (const auto& neighbor : neighbors)
    {
      // D_pk Ci
      const auto i0 = neighbor.i0;
      const auto i1 = neighbor.i1;

      const auto& p0 = particles[i0].position;
      const auto& p1 = particles[i1].position;

      const auto grad0 = 1.f / rho0_ * particles[i1].mass * gradPoly6(p0 - p1, h);
      const auto grad1 = -1.f / rho0_ * particles[i1].mass * gradPoly6(p0 - p1, h);

      // Add to denominator
      incompressibilitySelfGrad_[i0] += grad0;
      incompressibilityDenoms_[i0] += glm::dot(grad1, grad1);
    }

    // Compte lambdas
    incompressibilityLambdas_.resize(n);
    for (int i = 0; i < n; i++)
    {
      const auto denom = incompressibilityDenoms_[i] + glm::dot(incompressibilitySelfGrad_[i], incompressibilitySelfGrad_[i]);
      incompressibilityLambdas_[i] = -incompressibility_[i] / denom;
    }

    // Compte delta p
    deltaP_.resize(n);
    for (int i = 0; i < n; i++)
      deltaP_[i] = glm::vec3(0.f);

    for (const auto& neighbor : neighbors)
    {
      const auto i0 = neighbor.i0;
      const auto i1 = neighbor.i1;

      const auto& p0 = particles[i0].position;
      const auto& p1 = particles[i1].position;

      deltaP_[i0] += 1.f / rho0_ * (incompressibilityLambdas_[i0] + incompressibilityLambdas_[i1]) * particles[i1].mass * gradPoly6(p0 - p1, h);
    }

    // Update positions
    for (int i = 0; i < n; i++)
      particles[i].position += deltaP_[i];

    // Update new velocities based on positions
    for (int i = 0; i < n; i++)
      particles[i].velocity = (particles[i].position - positions_[i]) / dt;
  }

  particlesGeometry_->update(*particles_);
}
}
}
