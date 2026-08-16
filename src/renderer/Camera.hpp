#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace solar::renderer {

class Camera final {
public:
    Camera();

    void orbit(float deltaX, float deltaY);
    void pan(float deltaX, float deltaY);
    void zoom(float wheelDelta);
    void moveLocal(const glm::vec3& direction, float deltaSeconds);
    void focus(const glm::vec3& target, float distance);
    void focusFromDirection(const glm::vec3& target,
                            const glm::vec3& directionFromTarget,
                            float distance);
    void followTarget(const glm::vec3& target) noexcept;

    [[nodiscard]] glm::mat4 getViewMatrix() const;
    [[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const;

    void setFieldOfView(float degrees) noexcept;
    void setClipPlanes(float nearPlane, float farPlane) noexcept;
    void setOrbitSensitivity(float degreesPerPixel) noexcept;

    [[nodiscard]] const glm::vec3& position() const noexcept;
    [[nodiscard]] const glm::vec3& target() const noexcept;
    [[nodiscard]] glm::vec3 forward() const;
    [[nodiscard]] glm::vec3 right() const;
    [[nodiscard]] glm::vec3 up() const;
    [[nodiscard]] float yaw() const noexcept;
    [[nodiscard]] float pitch() const noexcept;
    [[nodiscard]] float fieldOfView() const noexcept;
    [[nodiscard]] float nearPlane() const noexcept;
    [[nodiscard]] float farPlane() const noexcept;
    [[nodiscard]] float distance() const noexcept;
    [[nodiscard]] float orbitSensitivity() const noexcept;

private:
    void updatePositionFromOrbit();
    void updateAnglesFromView();

    glm::vec3 m_position{8.0F, 6.0F, 8.0F};
    glm::vec3 m_target{0.0F};
    glm::vec3 m_worldUp{0.0F, 1.0F, 0.0F};
    float m_yaw{-135.0F};
    float m_pitch{-28.0F};
    float m_distance{12.8F};
    float m_fieldOfView{60.0F};
    float m_nearPlane{0.01F};
    float m_farPlane{10'000.0F};
    float m_orbitSensitivity{0.22F};
};

} // namespace solar::renderer
