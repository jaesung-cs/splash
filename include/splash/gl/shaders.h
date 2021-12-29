#ifndef SPLASH_GL_SHADERS_H_
#define SPLASH_GL_SHADERS_H_

#include <unordered_map>
#include <string>

namespace splash
{
namespace gl
{
class Shader;

class Shaders
{
public:
  Shaders();
  ~Shaders();

  Shader& operator [] (const std::string& name);

private:
  std::unordered_map<std::string, Shader> shaders_;
};
}
}

#endif // SPLASH_GL_SHADERS_H_
