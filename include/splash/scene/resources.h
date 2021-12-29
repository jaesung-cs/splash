#ifndef SPLASH_SCENE_RESOURCES_H_
#define SPLASH_SCENE_RESOURCES_H_

#include <memory>
#include <vector>

#include <splash/model/light.h>

namespace splash
{
namespace gl
{
class Texture;
class Geometry;
}

namespace model
{
class Camera;
}

namespace scene
{
class Resources
{
public:
  Resources();
  ~Resources();

  auto& floorTexture() const { return *floorTexture_; }
  auto& floorGeometry() const { return *floorGeometry_; }
  auto& camera() { return *camera_; }
  auto& lights() { return lights_; }
  const auto& lights() const { return lights_; }

private:
  std::unique_ptr<gl::Texture> floorTexture_;
  std::unique_ptr<gl::Geometry> floorGeometry_;
  std::unique_ptr<model::Camera> camera_;
  std::vector<model::Light> lights_;
};
}
}

#endif // SPLASH_SCENE_RESOURCES_H_
