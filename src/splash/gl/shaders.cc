#include <splash/gl/shaders.h>

#include <stdexcept>

#include <splash/gl/shader.h>

namespace splash
{
namespace gl
{
Shaders::Shaders()
{
  const std::string baseDirectory = "C:\\workspace\\splash\\src\\splash\\shader";

  shaders_.emplace("floor", Shader(baseDirectory, "floor"));
  shaders_.emplace("particles", Shader(baseDirectory, "particles"));
}

Shaders::~Shaders()
{
}

Shader& Shaders::operator [] (const std::string& name)
{
  auto it = shaders_.find(name);
  if (it != shaders_.cend())
    return it->second;
  throw std::runtime_error("Unregisted shader: " + name);
}
}
}
