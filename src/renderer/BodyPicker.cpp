#include "renderer/BodyPicker.hpp"

#include "renderer/Camera.hpp"

#include <glm/geometric.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/vec4.hpp>

#include <cmath>
#include <limits>

namespace solar::renderer {

std::optional<int> pickNearestSphere(
    const Camera& camera,
    const float aspectRatio,
    const glm::vec2& normalizedViewportPosition,
    const std::span<const PickableSphere> spheres) noexcept
{
    if (!(aspectRatio > 0.0F) || !std::isfinite(aspectRatio) ||
        normalizedViewportPosition.x < 0.0F || normalizedViewportPosition.x > 1.0F ||
        normalizedViewportPosition.y < 0.0F || normalizedViewportPosition.y > 1.0F) {
        return std::nullopt;
    }

    const float ndcX = normalizedViewportPosition.x * 2.0F - 1.0F;
    const float ndcY = 1.0F - normalizedViewportPosition.y * 2.0F;
    const glm::mat4 inverseViewProjection = glm::inverse(
        camera.getProjectionMatrix(aspectRatio) * camera.getViewMatrix());
    const glm::vec4 farHomogeneous = inverseViewProjection * glm::vec4{ndcX, ndcY, 1.0F, 1.0F};
    if (std::abs(farHomogeneous.w) <= 1.0e-6F || !std::isfinite(farHomogeneous.w)) {
        return std::nullopt;
    }

    const glm::vec3 farPoint = glm::vec3{farHomogeneous} / farHomogeneous.w;
    const glm::vec3 rayVector = farPoint - camera.position();
    const float rayLength = glm::length(rayVector);
    if (!(rayLength > 1.0e-6F) || !std::isfinite(rayLength)) return std::nullopt;
    const glm::vec3 rayDirection = rayVector / rayLength;

    std::optional<int> picked;
    float nearestDistance = std::numeric_limits<float>::max();
    for (const auto& sphere : spheres) {
        if (!(sphere.radius > 0.0F) || !std::isfinite(sphere.radius)) continue;
        const glm::vec3 fromCenter = camera.position() - sphere.center;
        const float projected = glm::dot(fromCenter, rayDirection);
        const float discriminant = projected * projected -
            (glm::dot(fromCenter, fromCenter) - sphere.radius * sphere.radius);
        if (discriminant < 0.0F || !std::isfinite(discriminant)) continue;

        const float root = std::sqrt(discriminant);
        float distance = -projected - root;
        if (distance < 0.0F) distance = -projected + root;
        if (distance >= 0.0F && distance < nearestDistance) {
            nearestDistance = distance;
            picked = sphere.identifier;
        }
    }
    return picked;
}

} // namespace solar::renderer
