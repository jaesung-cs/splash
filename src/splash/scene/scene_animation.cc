#include <splash/scene/scene_animation.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace splash
{
namespace scene
{
SceneAnimation::SceneAnimation()
  : Scene()
{
}

SceneAnimation::~SceneAnimation() = default;

std::string SceneAnimation::name() const
{
  return "Animation";
}

void SceneAnimation::drawUi()
{
  ImGui::Text("Animation");
}

void SceneAnimation::draw()
{
}
}
}
