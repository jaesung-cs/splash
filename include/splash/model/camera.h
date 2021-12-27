#ifndef SPLASH_MODEL_CAMERA_H_
#define SPLASH_MODEL_CAMERA_H_

#include <glm/glm.hpp>

namespace splash
{
namespace model
{
class Camera
{
public:
  Camera() = delete;
  Camera(float fov, float aspect);
  ~Camera();

  void setAspect(float aspect) noexcept { aspect_ = aspect; }
  auto distance() const noexcept { return distance_; }

  // Mouse control
  void translateByPixels(int dx, int dy);
  void rotateByPixels(int dx, int dy);
  void zoomByPixels(int dx, int dy);
  void zoomByScroll(float dx, float dy);

  // Keyboard control
  void moveForward(float distance);
  void moveRight(float distance);
  void moveUp(float distance);

  glm::vec3 eye() const;
  glm::mat4 view() const;
  glm::mat4 projection() const;

private:
  static constexpr float pi_ = 3.1415926535897932384626433832795;

  glm::vec3 center_{ 0.f, 0.f, 0.f };
  glm::vec3 up_{ 0.f, 0.f, 1.f };
  float distance_ = 1.f;

  float theta_ = 0.f;
  float phi_ = pi_ / 4.f;

  float fov_ = 60.f / 180.f * pi_;
  float aspect_ = 1.f;

  static constexpr float near_ = 0.01f;
  static constexpr float far_ = 100.f;

  float zoomSensitivity_ = 0.001f;
  float zoomScrollSensitivity_ = 0.1f;
  float translationSensitivity_ = 0.003f;
  float rotationSensitivity_ = 0.003f;
};
}
}

#endif // SPLASH_MODEL_CAMERA_H_
