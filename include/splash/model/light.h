#ifndef SPLASH_MODEL_LIGHT_H_
#define SPLASH_MODEL_LIGHT_H_

#include <glm/glm.hpp>

namespace splash
{
namespace model
{
struct Light
{
  alignas(16) glm::vec4 position{ 0.f, 0.f, 1.f, 0.f };
  glm::vec4 ambient{ 0.f };
  glm::vec4 diffuse{ 0.f };
  glm::vec4 specular{ 0.f };
};
}
}

#endif // SPLASH_MODEL_LIGHT_H_
