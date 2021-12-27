#include <splash/gl/geometry.h>

#include <glad/glad.h>

namespace splash
{
namespace gl
{
Geometry::Geometry(
  const std::vector<float>& vertex,
  const std::vector<uint32_t>& index,
  std::initializer_list<Attribute> attributes)
  : indexCount_(index.size())
{
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vertexBuffer_);
  glGenBuffers(1, &indexBuffer_);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);

  glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(float), vertex.data(), GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(uint32_t), index.data(), GL_STATIC_DRAW);

  for (const auto& attribute : attributes)
  {
    glVertexAttribPointer(attribute.index, attribute.size, GL_FLOAT, GL_FALSE, attribute.stride * sizeof(float), reinterpret_cast<void*>(attribute.offset * sizeof(float)));
    glEnableVertexAttribArray(attribute.index);
  }

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Geometry::~Geometry()
{
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vertexBuffer_);
  glDeleteBuffers(1, &indexBuffer_);
}

void Geometry::draw()
{
  glBindVertexArray(vao_);
  glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
}
}
