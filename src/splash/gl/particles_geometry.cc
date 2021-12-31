#include <splash/gl/particles_geometry.h>

#include <glad/glad.h>

#include <splash/geom/particle.h>
#include <splash/geom/particles.h>

namespace splash
{
namespace gl
{
ParticlesGeometry::ParticlesGeometry(uint32_t n)
  : particleCount_(n)
{
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vertexBuffer_);
  glGenBuffers(1, &indexBuffer_);
  glGenBuffers(1, &instanceBuffer_);

  // Create sphere buffer and index
  constexpr uint32_t segments = 8;
  constexpr float pi = 3.1415926535897932384626433832795f;
  std::vector<float> buffer;
  std::vector<uint32_t> index;

  for (int i = 0; i <= 2 * segments; i++)
  {
    const auto u = static_cast<float>(i) / (2 * segments);
    const auto theta = u * 2.f * pi;
    const auto cosTheta = std::cos(theta);
    const auto sinTheta = std::sin(theta);
    for (int j = 0; j <= segments; j++)
    {
      const auto v = static_cast<float>(j) / segments;
      const auto phi = (v - 0.5f) * pi;
      const auto cosPhi = std::cos(phi);
      const auto sinPhi = std::sin(phi);

      const glm::vec3 p(cosTheta * cosPhi, sinTheta * cosPhi, sinPhi);
      buffer.push_back(p.x);
      buffer.push_back(p.y);
      buffer.push_back(p.z);
    }
  }

  for (int i = 0; i < 2 * segments; i++)
  {
    const auto i0 = i * (segments + 1);
    const auto i1 = ((i + 1) % (2 * segments)) * (segments + 1);

    index.push_back(i0);
    index.push_back(i0 + 1);
    index.push_back(i1 + 1);

    for (int j = 1; j < segments; j++)
    {
      index.push_back(i0 + j);
      index.push_back(i0 + j + 1);
      index.push_back(i1 + j + 1);

      index.push_back(i0 + j);
      index.push_back(i1 + j + 1);
      index.push_back(i1 + j);
    }

    index.push_back(i0 + segments);
    index.push_back(i1 + segments);
    index.push_back(i1 + segments + 1);
  }

  indexCount_ = index.size();

  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
  glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(uint32_t), index.data(), GL_STATIC_DRAW);

  // Allocate instance buffer data
  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer_);
  glBufferData(GL_ARRAY_BUFFER, n * sizeof(geom::Particle), NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
  glVertexAttribDivisor(1, 1);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
  glVertexAttribDivisor(2, 1);
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

ParticlesGeometry::~ParticlesGeometry()
{
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vertexBuffer_);
  glDeleteBuffers(1, &instanceBuffer_);
  glDeleteBuffers(1, &instanceBuffer_);
}

void ParticlesGeometry::update(const geom::Particles& particles)
{
  particleCount_ = particles.size();

  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, particleCount_ * sizeof(geom::Particle), &particles[0]);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticlesGeometry::draw()
{
  glBindVertexArray(vao_);
  glDrawElementsInstanced(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0, particleCount_);
  glBindVertexArray(0);
}
}
}
