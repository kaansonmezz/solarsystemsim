#include "renderer/Camera.hpp"

#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <algorithm>
#include <cmath>

namespace solar::renderer {

Camera::Camera()
{
    updateAnglesFromView();
}

void Camera::orbit(const float deltaX, const float deltaY)
{
    m_yaw = std::remainder(m_yaw + deltaX * m_orbitSensitivity, 360.0F);
    m_pitch = std::clamp(m_pitch - deltaY * m_orbitSensitivity, -89.0F, 89.0F);
    updatePositionFromOrbit();
}

void Camera::pan(const float deltaX, const float deltaY)
{
    const float scale = std::max(m_distance, 0.1F) * 0.0015F;
    const glm::vec3 offset = (-right() * deltaX + up() * deltaY) * scale;
    m_position += offset;
    m_target += offset;
}

void Camera::zoom(const float wheelDelta)
{
    m_distance = std::clamp(
        m_distance * std::pow(0.88F, wheelDelta), 0.05F, 50'000.0F);
    updatePositionFromOrbit();
}

void Camera::moveLocal(const glm::vec3& direction, const float deltaSeconds)
{
    if (glm::dot(direction, direction) < 0.0001F) {
        return;
    }

    const float speed = std::max(2.0F, m_distance * 0.75F);
    const glm::vec3 movement = glm::normalize(
        right() * direction.x + m_worldUp * direction.y + forward() * direction.z) *
        speed * deltaSeconds;
    m_position += movement;
    m_target += movement;
}

void Camera::focus(const glm::vec3& target, const float distance)
{
    m_target = target;
    m_distance = std::clamp(distance, 0.05F, 50'000.0F);
    updatePositionFromOrbit();
}

void Camera::focusFromDirection(
    const glm::vec3& target,
    const glm::vec3& directionFromTarget,
    const float distance)
{
    const float directionLength = glm::length(directionFromTarget);
    if (!(directionLength > 1.0e-6F) || !std::isfinite(directionLength)) return;
    m_target = target;
    m_distance = std::clamp(distance, 0.05F, 50'000.0F);
    m_position = m_target + directionFromTarget / directionLength * m_distance;
    updateAnglesFromView();
}

void Camera::followTarget(const glm::vec3& target) noexcept
{
    const glm::vec3 offset = target - m_target;
    m_target = target;
    m_position += offset;
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(m_position, m_target, m_worldUp);
}

glm::mat4 Camera::getProjectionMatrix(const float aspectRatio) const
{
    return glm::perspective(
        glm::radians(m_fieldOfView), std::max(aspectRatio, 0.001F), m_nearPlane, m_farPlane);
}

void Camera::setFieldOfView(const float degrees) noexcept
{
    m_fieldOfView = std::clamp(degrees, 20.0F, 120.0F);
}

void Camera::setClipPlanes(const float nearPlane, const float farPlane) noexcept
{
    m_nearPlane = std::clamp(nearPlane, 0.0001F, 100.0F);
    m_farPlane = std::max(farPlane, m_nearPlane + 1.0F);
}

void Camera::setOrbitSensitivity(const float degreesPerPixel) noexcept
{
    m_orbitSensitivity = std::clamp(degreesPerPixel, 0.01F, 2.0F);
}

const glm::vec3& Camera::position() const noexcept { return m_position; }
const glm::vec3& Camera::target() const noexcept { return m_target; }

glm::vec3 Camera::forward() const
{
    return glm::normalize(m_target - m_position);
}

glm::vec3 Camera::right() const
{
    return glm::normalize(glm::cross(forward(), m_worldUp));
}

glm::vec3 Camera::up() const
{
    return glm::normalize(glm::cross(right(), forward()));
}

float Camera::yaw() const noexcept { return m_yaw; }
float Camera::pitch() const noexcept { return m_pitch; }
float Camera::fieldOfView() const noexcept { return m_fieldOfView; }
float Camera::nearPlane() const noexcept { return m_nearPlane; }
float Camera::farPlane() const noexcept { return m_farPlane; }
float Camera::distance() const noexcept { return m_distance; }
float Camera::orbitSensitivity() const noexcept { return m_orbitSensitivity; }

void Camera::updatePositionFromOrbit()
{
    const float yawRadians = glm::radians(m_yaw);
    const float pitchRadians = glm::radians(m_pitch);
    const glm::vec3 fromTarget{
        std::cos(pitchRadians) * std::cos(yawRadians),
        std::sin(pitchRadians),
        std::cos(pitchRadians) * std::sin(yawRadians),
    };
    m_position = m_target - fromTarget * m_distance;
}

void Camera::updateAnglesFromView()
{
    const glm::vec3 direction = glm::normalize(m_target - m_position);
    m_pitch = glm::degrees(std::asin(direction.y));
    m_yaw = glm::degrees(std::atan2(direction.z, direction.x));
    m_distance = glm::distance(m_position, m_target);
}

} // namespace solar::renderer
