#ifndef SPLASH_SCENE_SCENE_ANIMATION_H_
#define SPLASH_SCENE_SCENE_ANIMATION_H_

#include <splash/scene/scene.h>

namespace splash
{
namespace scene
{
class SceneAnimation final : public Scene
{
public:
  SceneAnimation();
  ~SceneAnimation() override;

  std::string name() const override;

  void drawUi() override;
  void draw() override;

private:
};
}
}

#endif // SPLASH_SCENE_SCENE_ANIMATION_H_
