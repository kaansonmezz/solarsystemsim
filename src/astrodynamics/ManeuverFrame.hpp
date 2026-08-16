#pragma once

#include "astronomy/StateVector.hpp"

#include <glm/vec3.hpp>
#include <optional>

namespace solar::astrodynamics {

// Orthonormal RTN frame: radial-out follows r, normal follows r x v, and tangential
// follows N x R. "Prograde" UI controls use this local along-track direction; it is
// intentionally not normalize(v) when an eccentric state has radial velocity.
struct ManeuverFrame final {
    glm::dvec3 tangential;
    glm::dvec3 normal;
    glm::dvec3 radialOut;
};

[[nodiscard]] std::optional<ManeuverFrame> makeManeuverFrame(
    const astronomy::StateVector& state) noexcept;

} // namespace solar::astrodynamics
