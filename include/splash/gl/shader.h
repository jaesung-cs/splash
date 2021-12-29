#ifndef SPLASH_GL_SHADER_H_
#define SPLASH_GL_SHADER_H_

#include <string>
#include <cstdint>

#include <glm/fwd.hpp>

namespace splash
{
namespace gl
{
class Shader
{
public:
  Shader() = delete;
  Shader(const std::string& dirpath, const std::string& name);
  ~Shader();

  Shader(const Shader& rhs) = delete;
  Shader& operator = (const Shader& rhs) = delete;

  Shader(Shader&& rhs) noexcept
  {
    program_ = rhs.program_;
    rhs.program_ = 0;
  }

  Shader& operator = (Shader&& rhs) noexcept
  {
    program_ = rhs.program_;
    rhs.program_ = 0;
    return *this;
  }

  void use();
  void done();

  void uniform1i(const std::string& name, int v);
  void uniform1f(const std::string& name, float v);
  void uniform3f(const std::string& name, const glm::vec3& v);
  void uniform4f(const std::string& name, const glm::vec4& v);
  void uniformMatrix3f(const std::string& name, const glm::mat3& m);
  void uniformMatrix4f(const std::string& name, const glm::mat4& m);

private:
  uint32_t location(const std::string& name);

  uint32_t program_ = 0;
};
}
}

#endif // SPLASH_GL_SHADER_H_
