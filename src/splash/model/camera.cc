#include <splash/model/camera.h>

#include <cmath>
#include <algorithm>

#include <glm/gtx/transform.hpp>

namespace splash
{
namespace model
{
Camera::Camera(float fov, float aspect)
  : fov_(fov)
  , aspect_(aspect)
{
}

Camera::~Camera()
{
}

void Camera::translateByPixels(int dx, int dy)
{
  const auto cosTheta = std::cos(theta_);
  const auto sinTheta = std::sin(theta_);
  const auto cosPhi = std::cos(phi_);
  const auto sinPhi = std::sin(phi_);

  center_ += translationSensitivity_ * distance_ * (
    static_cast<float>(dx) * glm::vec3(sinTheta, -cosTheta, 0.f)
    + -static_cast<float>(dy) * glm::vec3(cosTheta, sinTheta, -cosPhi));
}

void Camera::rotateByPixels(int dx, int dy)
{
  theta_ += -rotationSensitivity_ * dx;

  constexpr float clip = 1.f / 180.f * pi_;
  phi_ = std::clamp(phi_ + rotationSensitivity_ * dy, -pi_ / 2 + clip, pi_ / 2 - clip);
}

void Camera::zoomByPixels(int dx, int dy)
{
  distance_ *= std::exp(zoomSensitivity_ * dy);
}

void Camera::zoomByScroll(float dx, float dy)
{
  distance_ *= std::exp(-zoomScrollSensitivity_ * dy);
}

void Camera::moveForward(float distance)
{
  const auto cosTheta = std::cos(theta_);
  const auto sinTheta = std::sin(theta_);
  const auto cosPhi = std::cos(phi_);
  const auto sinPhi = std::sin(phi_);

  center_ += -distance * glm::vec3(cosTheta * cosPhi, sinTheta * cosPhi, sinPhi);
}

void Camera::moveRight(float distance)
{
  const auto cosTheta = std::cos(theta_);
  const auto sinTheta = std::sin(theta_);
  const auto cosPhi = std::cos(phi_);
  const auto sinPhi = std::sin(phi_);

  center_ += -distance * glm::vec3(sinTheta, -cosTheta, 0.f);
}

void Camera::moveUp(float distance)
{
  center_ += distance * up_;
}

glm::vec3 Camera::eye() const
{
  const auto cosTheta = std::cos(theta_);
  const auto sinTheta = std::sin(theta_);
  const auto cosPhi = std::cos(phi_);
  const auto sinPhi = std::sin(phi_);

  return center_ + distance_ * glm::vec3(cosTheta * cosPhi, sinTheta * cosPhi, sinPhi);
}

glm::mat4 Camera::view() const
{
  return glm::lookAt(eye(), center_, up_);
}

glm::mat4 Camera::projection() const
{
  return glm::perspective(fov_, aspect_, near_, far_);
}
}
}
