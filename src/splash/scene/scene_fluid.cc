#include <splash/scene/scene_fluid.h>

#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define NOMINMAX
#include <tbb/tbb.h>

#include <splash/gl/shaders.h>
#include <splash/gl/shader.h>
#include <splash/gl/texture.h>
#include <splash/gl/geometry.h>
#include <splash/gl/particles_geometry.h>
#include <splash/geom/particles.h>
#include <splash/model/camera.h>
#include <splash/scene/resources.h>
#include <splash/fluid/neighbor_search_spatial_hashing.h>
#include <splash/fluid/sph_kernel.h>

namespace splash
{
namespace scene
{
SceneFluid::SceneFluid(Resources* resources, gl::Shaders* shaders)
  : Scene()
  , resources_(resources)
  , shaders_(shaders)
{
  particles_ = std::make_unique<geom::Particles>(maxParticleCount_);
  fluidParticles_ = std::make_unique<geom::Particles>(maxFluidCount_);
  particlesGeometry_ = std::make_unique<gl::ParticlesGeometry>(maxParticleCount_);

  lastTime_ = std::chrono::high_resolution_clock::now();

  neighborSearch_ = std::make_unique<fluid::NeighborSearchSpatialHashing>();

  initializeParticles();
}

SceneFluid::~SceneFluid() = default;

void SceneFluid::drawUi()
{
  ImGui::SliderInt("X", &fluidSideX_, 1, maxFluidSide_);
  ImGui::SliderInt("Y", &fluidSideY_, 1, maxFluidSide_);
  ImGui::SliderInt("Z", &fluidSideZ_, 1, maxFluidSide_);

  if (ImGui::Button("Initialize"))
  {
    waveAnimationTime_ = 0.f;
    initializeParticles();
  }

  ImGui::Text("%d fluid particles", fluidCount_);
  ImGui::Text("%d boundary particles", particleCount_ - fluidCount_);
  ImGui::Text("%d total particles", particleCount_);

  ImGui::Checkbox("Show boundary", &showBoundary_);

  ImGui::Checkbox("Animation", &animation_);

  ImGui::Checkbox("Multiprocessing", &multiprocessing_);

  static const std::vector<float> timestepScaleTable{
    1.f,
    1.7f,
    3.f,
    5.f,
    10.f,
  };
  ImGui::SliderInt("Slow motion", &timestepScaleLevel_, 0, timestepScaleTable.size() - 1, "");

  timestepScale_ = timestepScaleTable[timestepScaleLevel_];
  ImGui::Text("Animation speed X%.1lf", timestepScale_);

  ImGui::Checkbox("Wave", &wave_);
  ImGui::SliderFloat("Wave speed", &waveSpeed_, 0.f, 5.f);

  static std::vector<std::string> kernels{
    "Poly6",
    "Spiky",
    "Cubic",
  };

  ImGui::Text("Kernel");
  ImGui::PushID(0);
  for (int i = 0; i < kernels.size(); i++)
  {
    ImGui::SameLine();
    if (ImGui::RadioButton(kernels[i].c_str(), kernelIndex_ == i))
      kernelIndex_ = i;
  }
  ImGui::PopID();

  ImGui::Text("Gradient");
  ImGui::PushID(1);
  for (int i = 0; i < kernels.size(); i++)
  {
    ImGui::SameLine();
    if (ImGui::RadioButton(kernels[i].c_str(), gradKernelIndex_ == i))
      gradKernelIndex_ = i;
  }
  ImGui::PopID();

  ImGui::SliderFloat("Viscosity", &viscosity_, 0.f, 1.f);
}

void SceneFluid::draw()
{
  const auto now = std::chrono::high_resolution_clock::now();
  auto dt = std::chrono::duration<float>(now - lastTime_).count();
  lastTime_ = now;

  // Slow motion
  dt /= timestepScale_;

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
  constexpr float radius = 0.1f;

  particles.radius() = radius;

  fluidCount_ = fluidSideX_ * fluidSideY_ * fluidSideZ_;
  particleCount_ = fluidCount_ + (fluidSideX_ * 3 * fluidSideY_ + fluidSideY_ * fluidSideZ_ + fluidSideZ_ * fluidSideX_ * 3) * 2;
  particles.resize(particleCount_);

  // Kernels
  const auto h = radius * 4.f;
  kernels_.resize(3);
  kernels_[0] = std::make_unique<fluid::SphKernelPoly6>(h);
  kernels_[1] = std::make_unique<fluid::SphKernelSpiky>(h);
  kernels_[2] = std::make_unique<fluid::SphKernelSpiky>(h); // TODO: change to cubic

  rho0_ = 997.f;

  constexpr float pi = 3.1415926535897932384626433832795f;
  const auto mass = 0.8 * rho0_ * 8.f * radius * radius * radius; // Cubic particle

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
        particle.position = glm::vec3(i + 1, j + 1, k + 1) * 2.f * radius;
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

  int index = fluidCount_;
  for (int i = 0; i < fluidSideX_ * 3; i++)
  {
    for (int j = 0; j < fluidSideY_; j++)
    {
      const auto b = glm::vec3(i + 1, j + 1, 0.f) * 2.f * radius;

      boundaryParticle.position = glm::vec3(b.x, b.y, b.z);
      particles[index++] = boundaryParticle;

      boundaryParticle.position = glm::vec3(b.x, b.y, b.z + (fluidSideZ_ + 1) * 2.f * radius);
      particles[index++] = boundaryParticle;
    }
  }

  for (int i = 0; i < fluidSideX_ * 3; i++)
  {
    for (int j = 0; j < fluidSideZ_; j++)
    {
      const auto b = glm::vec3(i + 1, j + 1, 0.f) * 2.f * radius;

      boundaryParticle.position = glm::vec3(b.x, b.z, b.y);
      particles[index++] = boundaryParticle;

      boundaryParticle.position = glm::vec3(b.x, b.z + (fluidSideY_ + 1) * 2.f * radius, b.y);
      particles[index++] = boundaryParticle;
    }
  }

  for (int i = 0; i < fluidSideY_; i++)
  {
    for (int j = 0; j < fluidSideZ_; j++)
    {
      const auto b = glm::vec3(i + 1, j + 1, 0.f) * 2.f * radius;

      boundaryParticle.position = glm::vec3(b.z, b.x, b.y);
      boundaryParticle.velocity = { 1.f, 0.f, 0.f };
      particles[index++] = boundaryParticle;
      boundaryParticle.velocity = { 0.f, 0.f, 0.f };

      boundaryParticle.position = glm::vec3(b.z + (fluidSideX_ * 3 + 1) * 2.f * radius, b.x, b.y);
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

    // Boundary wave animation
    if (wave_)
    {
      waveAnimationTime_ += dt * waveSpeed_;
      constexpr float amplitude = 1.f;
      for (int i = 0; i < n; i++)
      {
        if (particles[i].type == geom::ParticleType::BOUNDARY && particles[i].velocity.x != 0.f)
          particles[i].position.x = (1.f - std::cos(waveAnimationTime_)) / 2.f * amplitude;
      }
    }

    const auto& kernel = *kernels_[kernelIndex_];
    const auto& gradKernel = *kernels_[gradKernelIndex_];

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
    {
      const auto h = 4.f * radius; // SPH support radius
      neighborSearch_->setMultiprocessing(multiprocessing_);
      neighborSearch_->computeNeighbors(particles, h);
      const auto& neighbors = neighborSearch_->neighbors();

      neighborIndices_.resize(n);
      forEach(0, n, [&](int i) {
        neighborIndices_[i].clear();
        });

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
    forEach(0, n1, [&](int i)
      {
        const auto i0 = boundaryIndices_[i];

        float delta = kernel(glm::vec3(0.f));

        for (auto i1 : neighborIndices_[i0])
        {
          if (particles[i1].type == geom::ParticleType::BOUNDARY)
          {
            const auto& p0 = particles[i0].position;
            const auto& p1 = particles[i1].position;

            delta += kernel(p0 - p1);
          }
        }

        const auto volume = 1.f / delta;

        // Update boundary particle mass
        particles[i0].mass = rho0_ * volume;
      });

    // Projection steps
    constexpr uint32_t maxSteps = 5;
    for (int step = 0; step < maxSteps; step++)
    {
      // Density calculation
      forEach(0, n0, [&](int i)
        {
          const auto i0 = fluidIndices_[i];

          // Contribution from self
          density_[i] = particles[i0].mass * kernel(glm::vec3(0.f));

          // Contribution from neighbors
          for (auto i1 : neighborIndices_[i0])
          {
            const auto& p0 = particles[i0].position;
            const auto& p1 = particles[i1].position;

            density_[i] += particles[i1].mass * kernel(p0 - p1);
          }
        });

      // Solve project to make incompressibility = 0
      forEach(0, n0, [&](int i)
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

              const glm::vec3 grad0 = 1.f / rho0_ * m1 * gradKernel.grad(p0 - p1);
              const glm::vec3 grad1 = -1.f / rho0_ * m1 * gradKernel.grad(p0 - p1);

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
        });

      // Compte delta p
      for (int i = 0; i < n0; i++)
        deltaP_[i] = glm::vec3(0.f);

      forEach(0, n0, [&](int i)
        {
          const auto i0 = fluidIndices_[i];

          for (auto i1 : neighborIndices_[i0])
          {
            const auto& p0 = particles[i0].position;
            const auto& p1 = particles[i1].position;

            const auto m1 = particles[i1].mass;

            if (particles[i1].type == geom::ParticleType::FLUID)
              deltaP_[i] += 1.f / rho0_ * (incompressibilityLambdas_[i] + incompressibilityLambdas_[toFluidIndex_[i1]]) * m1 * gradKernel.grad(p0 - p1);
            else
              deltaP_[i] += 1.f / rho0_ * incompressibilityLambdas_[i] * m1 * gradKernel.grad(p0 - p1);
          }
        });

      // Update positions
      forEach(0, n0, [&](int i)
        {
          const auto i0 = fluidIndices_[i];
          particles[i0].position += deltaP_[i];
        });
    }

    // Update velocity
    forEach(0, n0, [&](int i)
      {
        const auto i0 = fluidIndices_[i];
        particles[i0].velocity = (particles[i0].position - positions_[i0]) / dt;
      });

    // Solve viscosity
    for (int i = 0; i < n0; i++)
    {
      const auto i0 = fluidIndices_[i];

      for (auto i1 : neighborIndices_[i0])
      {
        if (particles[i1].type == geom::ParticleType::FLUID)
        {
          const auto& p0 = particles[i0].position;
          const auto& p1 = particles[i1].position;

          const auto& v0 = particles[i0].velocity;
          const auto& v1 = particles[i1].velocity;

          const auto m1 = particles[i1].mass;

          const auto density1 = density_[toFluidIndex_[i1]];

          particles[i0].velocity -= viscosity_ * (m1 / density1) * (v0 - v1) * kernel(p0 - p1);
        }
      }
    }
  }

  // Update color mapped with velocity
  constexpr float vmax = 3.f;
  for (int i = 0; i < particles.size(); i++)
  {
    if (particles[i].type == geom::ParticleType::FLUID)
    {
      const auto v2 = glm::dot(particles[i].velocity, particles[i].velocity);
      const auto v = std::sqrt(v2);
      const auto t = std::min(v / vmax, 1.f);
      particles[i].color = glm::vec3(t, t, 1.f);
    }
  }

  // Update with boundary visibility
  if (showBoundary_)
    particlesGeometry_->update(*particles_);
  else
  {
    updateFluidParticles();
    particlesGeometry_->update(*fluidParticles_);
  }
}

void SceneFluid::updateFluidParticles()
{
  fluidParticles_->radius() = particles_->radius();
  fluidParticles_->resize(fluidCount_);

  int index = 0;
  for (int i = 0; i < particles_->size(); i++)
  {
    if ((*particles_)[i].type == geom::ParticleType::FLUID)
      (*fluidParticles_)[index++] = (*particles_)[i];
  }
}

void SceneFluid::forEach(int begin, int end, std::function<void(int)> f)
{
  // TODO: multiprocessing
  if (multiprocessing_)
  {
    tbb::parallel_for(tbb::blocked_range<int>(begin, end),
      [&](const tbb::blocked_range<int>& range)
      {
        for (int i = range.begin(); i < range.end(); i++)
          f(i);
      });
  }
  else
  {
    for (int i = begin; i < end; i++)
      f(i);
  }
}
}
}
