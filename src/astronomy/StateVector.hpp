#pragma once

#include <glm/vec3.hpp>

namespace solar::astronomy {

// Scientific units: position in kilometres, velocity in kilometres per second.
struct StateVector final {
    glm::dvec3 positionKm{0.0};
    glm::dvec3 velocityKmPerSec{0.0};
};

[[nodiscard]] bool isFinite(const StateVector& state) noexcept;

} // namespace solar::astronomy

