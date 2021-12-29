#include <splash/scene/scene.h>

namespace splash
{
namespace scene
{
Scene::Scene() = default;

Scene::~Scene() = default;

std::string Scene::name() const
{
  return "(empty)";
}

void Scene::drawUi()
{
}

void Scene::draw()
{
}
}
}
