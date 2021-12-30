#include <splash/gl/boxes_geometry.h>

#include <glad/glad.h>

#include <splash/model/box.h>

namespace splash
{
namespace gl
{
BoxesGeometry::BoxesGeometry(uint32_t size)
  : size_(size)
{
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  glGenBuffers(1, &vertexBuffer_);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
  std::vector<float> vertexBuffer{
    0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 1.f, 1.f, 0.f,
    0.f, 0.f, 1.f, 1.f, 0.f, 1.f,
    0.f, 1.f, 1.f, 1.f, 1.f, 1.f,

    0.f, 0.f, 0.f, 0.f, 1.f, 0.f,
    1.f, 0.f, 0.f, 1.f, 1.f, 0.f,
    0.f, 0.f, 1.f, 0.f, 1.f, 1.f,
    1.f, 0.f, 1.f, 1.f, 1.f, 1.f,

    0.f, 0.f, 0.f, 0.f, 0.f, 1.f,
    1.f, 0.f, 0.f, 1.f, 0.f, 1.f,
    0.f, 1.f, 0.f, 0.f, 1.f, 1.f,
    1.f, 1.f, 0.f, 1.f, 1.f, 1.f,
  };
  glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(float), vertexBuffer.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glGenBuffers(1, &indexBuffer_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
  std::vector<uint32_t> indexBuffer{
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23,
  };
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(uint32_t), indexBuffer.data(), GL_STATIC_DRAW);

  glGenBuffers(2, instanceBuffers_);
  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffers_[0]);
  glBufferData(GL_ARRAY_BUFFER, size_ * sizeof(float) * 6, 0, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
  glVertexAttribDivisor(1, 1);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
  glVertexAttribDivisor(2, 1);
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffers_[1]);
  glBufferData(GL_ARRAY_BUFFER, size_ * sizeof(float) * 3, 0, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glVertexAttribDivisor(3, 1);
  glEnableVertexAttribArray(3);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  indexCount_ = indexBuffer.size();
}

BoxesGeometry::~BoxesGeometry()
{
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vertexBuffer_);
  glDeleteBuffers(1, &indexBuffer_);
  glDeleteBuffers(2, instanceBuffers_);
}

void BoxesGeometry::update(const std::vector<model::Box>& boxes, const glm::vec3& color)
{
  const auto n = boxes.size();
  std::vector<glm::vec3> colors(n, color);

  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffers_[0]);
  glBufferSubData(GL_ARRAY_BUFFER, 0, n * sizeof(float) * 6, boxes.data());

  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffers_[1]);
  glBufferSubData(GL_ARRAY_BUFFER, 0, n * sizeof(float) * 3, colors.data());

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  instanceCount_ = n;
}

void BoxesGeometry::draw()
{
  glBindVertexArray(vao_);
  glDrawElementsInstanced(GL_LINES, indexCount_, GL_UNSIGNED_INT, 0, instanceCount_);
  glBindVertexArray(0);
}
}
}
