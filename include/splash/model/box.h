#ifndef SPLASH_MODEL_BOX_H_
#define SPLASH_MODEL_BOX_H_

#include <glm/glm.hpp>

namespace splash
{
namespace model
{
struct Box
{
  glm::vec3 min;
  glm::vec3 max;
};
}
}

#endif // SPLASH_MODEL_BOX_H_
