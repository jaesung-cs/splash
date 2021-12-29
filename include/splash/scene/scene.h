#ifndef SPLASH_SCENE_SCENE_H_
#define SPLASH_SCENE_SCENE_H_

#include <string>
#include <memory>

namespace splash
{
namespace scene
{
class Scene
{
public:
  Scene();
  virtual ~Scene();

  virtual std::string name() const;

  virtual void drawUi();
  virtual void draw();

private:
};
}
}

#endif // SPLASH_SCENE_SCENE_H_
