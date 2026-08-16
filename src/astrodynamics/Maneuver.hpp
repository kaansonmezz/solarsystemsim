#pragma once

#include "astrodynamics/Orbit.hpp"
#include "astronomy/StateVector.hpp"

#include <glm/vec3.hpp>
#include <optional>

namespace solar::astrodynamics {

// Scientific storage is km/s. UI converts to/from the user-facing m/s values.
struct ImpulsiveManeuver final {
    double tangentialKmPerSec{};
    double normalKmPerSec{};
    double radialKmPerSec{};

    [[nodiscard]] double magnitudeKmPerSec() const noexcept;
};

[[nodiscard]] std::optional<glm::dvec3> maneuverDeltaVelocity(
    const astronomy::StateVector& state,
    const ImpulsiveManeuver& maneuver) noexcept;
[[nodiscard]] std::optional<astronomy::StateVector> applyManeuver(
    const astronomy::StateVector& state,
    const ImpulsiveManeuver& maneuver) noexcept;
[[nodiscard]] std::optional<double> circularizationDeltaVelocityKmPerSec(
    const astronomy::StateVector& state,
    double muKm3PerSec2) noexcept;
// Tangential burn at one apsis to set the radius of the opposite apsis.
[[nodiscard]] std::optional<double> apsidalTargetDeltaVelocityKmPerSec(
    const astronomy::StateVector& state,
    double targetOppositeApsisRadiusKm,
    double muKm3PerSec2) noexcept;

} // namespace solar::astrodynamics
