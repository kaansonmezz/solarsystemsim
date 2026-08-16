#pragma once

#include <optional>
#include <span>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace solar::renderer {

class Camera;

struct PickableSphere final {
    int identifier{0};
    glm::vec3 center{0.0F};
    float radius{0.0F};
};

[[nodiscard]] std::optional<int> pickNearestSphere(
    const Camera& camera,
    float aspectRatio,
    const glm::vec2& normalizedViewportPosition,
    std::span<const PickableSphere> spheres) noexcept;

} // namespace solar::renderer
