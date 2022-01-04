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
  constexpr float radius = 1.f / fluidSideX_;

  particles.radius() = radius;

  rho0_ = 997.f;

  constexpr float pi = 3.1415926535897932384626433832795f;
  const auto mass = rho0_ * 8.f * radius * radius * radius; // Cubic particle

  constexpr float baseHeight = 0.2f;

  for (int i = 0; i < fluidSideX_; i++)
  {
    for (int j = 0; j < fluidSideY_; j++)
    {
      for (int k = 0; k < fluidSideZ_; k++)
      {
        const auto index = i * fluidSideY_ * fluidSideZ_ + j * fluidSideZ_ + k;

        auto& particle = particles[index];
        particle.type = geom::ParticleType::FLUID;
        particle.position = glm::vec3(i + k * 0.1f, j + k * 0.05f, k * 0.8f) * 2.5f * radius + glm::vec3(0.f, 0.f, baseHeight);
        particle.mass = mass;
        particle.velocity = { 0.f, 0.f, 0.f };
        particle.color = { 0.f, 0.f, 1.f };
      }
    }
  }

  // Boundary generation
  geom::Particle boundaryParticle;
  boundaryParticle.type = geom::ParticleType::BOUNDARY;
  boundaryParticle.mass = 0.f;
  boundaryParticle.color = glm::vec3(101.f, 67.f, 33.f) / 255.f;
  boundaryParticle.velocity = { 0.f, 0.f, 0.f };

  constexpr glm::vec3 baseOffset(-0.2f, -0.2f, radius);
  const auto boundaryLength = boundarySide_ * 2.f * radius;
  int index = fluidCount_;
  for (int i = 0; i < boundarySide_; i++)
  {
    for (int j = 0; j < boundarySide_; j++)
    {
      const auto b = glm::vec3(i + 0.5f, j + 0.5f, 0.f) * 2.f * radius;

      boundaryParticle.position = baseOffset + glm::vec3(b.x, b.y, b.z);
      particles[index++] = boundaryParticle;

      boundaryParticle.position = baseOffset + glm::vec3(b.x, b.y, b.z + boundaryLength);
      particles[index++] = boundaryParticle;

      boundaryParticle.position = baseOffset + glm::vec3(b.x, b.z, b.y);
      particles[index++] = boundaryParticle;

      boundaryParticle.position = baseOffset + glm::vec3(b.x, b.z + boundaryLength, b.y);
      particles[index++] = boundaryParticle;

      boundaryParticle.position = baseOffset + glm::vec3(b.z, b.x, b.y);
      particles[index++] = boundaryParticle;

      boundaryParticle.position = baseOffset + glm::vec3(b.z + boundaryLength, b.x, b.y);
      particles[index++] = boundaryParticle;
    }
  }
}

void SceneFluid::updateParticles(float dt)
{
  auto& particles = *particles_;
  const auto radius = particles_->radius();

  if (animation_)
  {
    const auto n = particles.size();

    constexpr glm::vec3 gravity = { 0.f, 0.f, -9.80665f };
    positions_.resize(n);
    for (int i = 0; i < n; i++)
    {
      // Store old particle positions
      positions_[i] = particles[i].position;

      // Update particles
      if (particles[i].type == geom::ParticleType::FLUID)
      {
        particles[i].velocity += gravity * dt;
        particles[i].position += particles[i].velocity * dt;
      }
    }

    // Neighbor search
    const auto h = 4.f * radius; // SPH support radius
    {
      neighborSearch_->computeNeighbors(particles, h);
      const auto& neighbors = neighborSearch_->neighbors();

      neighborIndices_.resize(n);
      for (int i = 0; i < n; i++)
        neighborIndices_[i].clear();

      for (auto neighbor : neighbors)
      {
        const auto i0 = neighbor.i0;
        const auto i1 = neighbor.i1;

        neighborIndices_[i0].push_back(i1);
      }
    }

    // TODO: Move fluid simulation to a class
    // Split fluid and boundary
    fluidIndices_.clear();
    boundaryIndices_.clear();
    toFluidIndex_.resize(n);
    for (int i = 0; i < n; i++)
    {
      if (particles[i].type == geom::ParticleType::FLUID)
      {
        toFluidIndex_[i] = fluidIndices_.size();
        fluidIndices_.push_back(i);
      }
      else
      {
        toFluidIndex_[i] = -1;
        boundaryIndices_.push_back(i);
      }
    }

    const auto n0 = fluidIndices_.size();
    const auto n1 = boundaryIndices_.size();

    density_.resize(n0);
    incompressibilityLambdas_.resize(n0);
    deltaP_.resize(n0);

    // Compute boundary psi
    for (int i = 0; i < n1; i++)
    {
      const auto i0 = boundaryIndices_[i];

      float delta = Poly6(glm::vec3(0.f), h);

      for (auto i1 : neighborIndices_[i0])
      {
        if (particles[i1].type == geom::ParticleType::BOUNDARY)
        {
          const auto& p0 = particles[i0].position;
          const auto& p1 = particles[i1].position;

          delta += Poly6(p0 - p1, h);
        }
      }

      const auto volume = 1.f / delta;

      // Update boundary particle mass
      particles[i0].mass = rho0_ * volume;
    }

    // Projection steps
    constexpr uint32_t maxSteps = 5;
    for (int step = 0; step < maxSteps; step++)
    {
      // Density calculation
      for (int i = 0; i < n0; i++)
      {
        const auto i0 = fluidIndices_[i];

        // Contribution from self
        density_[i] = particles[i0].mass * Poly6(glm::vec3(0.f), h);

        // Contribution from neighbors
        for (auto i1 : neighborIndices_[i0])
        {
          const auto& p0 = particles[i0].position;
          const auto& p1 = particles[i1].position;

          density_[i] += particles[i1].mass * Poly6(p0 - p1, h);
        }
      }

      // Solve project to make incompressibility = 0
      for (int i = 0; i < n0; i++)
      {
        const auto i0 = fluidIndices_[i];

        const auto incompressibility = std::max(density_[i] / rho0_ - 1.f, 0.f);
        if (incompressibility > 0.f)
        {
          glm::vec3 selfGrad(0.f);
          float denom = 0.f;

          for (auto i1 : neighborIndices_[i0])
          {
            const auto& p0 = particles[i0].position;
            const auto& p1 = particles[i1].position;

            const auto m1 = particles[i1].mass;

            const glm::vec3 grad0 = 1.f / rho0_ * m1 * gradPoly6(p0 - p1, h);
            const glm::vec3 grad1 = -1.f / rho0_ * m1 * gradPoly6(p0 - p1, h);

            // Add to gradient by self
            selfGrad += grad0;

            // Add to denominator for movable fluid particles
            if (particles[i1].type == geom::ParticleType::FLUID)
              denom += glm::dot(grad1, grad1);
          }

          denom += glm::dot(selfGrad, selfGrad);

          // Compute lambdas
          incompressibilityLambdas_[i] = -incompressibility / denom;
        }
        else
          incompressibilityLambdas_[i] = 0.f;
      }

      // Compte delta p
      for (int i = 0; i < n0; i++)
        deltaP_[i] = glm::vec3(0.f);

      for (int i = 0; i < n0; i++)
      {
        const auto i0 = fluidIndices_[i];

        for (auto i1 : neighborIndices_[i0])
        {
          const auto& p0 = particles[i0].position;
          const auto& p1 = particles[i1].position;

          const auto m1 = particles[i1].mass;

          if (particles[i1].type == geom::ParticleType::FLUID)
            deltaP_[i] += 1.f / rho0_ * (incompressibilityLambdas_[i] + incompressibilityLambdas_[toFluidIndex_[i1]]) * m1 * gradPoly6(p0 - p1, h);
          else
            deltaP_[i] += 1.f / rho0_ * incompressibilityLambdas_[i] * m1 * gradPoly6(p0 - p1, h);
        }
      }

      // Update positions
      for (int i = 0; i < n0; i++)
      {
        const auto i0 = fluidIndices_[i];
        particles[i0].position += deltaP_[i];
      }
    }

    // Update velocity
    for (int i = 0; i < n0; i++)
    {
      const auto i0 = fluidIndices_[i];
      particles[i0].velocity = (particles[i0].position - positions_[i0]) / dt;
    }
  }

  particlesGeometry_->update(*particles_);
}
}
}
