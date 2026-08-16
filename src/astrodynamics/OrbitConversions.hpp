#pragma once

#include "astronomy/StateVector.hpp"
#include "astrodynamics/OrbitalElements.hpp"

#include <optional>

namespace solar::astrodynamics {

// Singular conventions:
// - equatorial orbit: RAAN = 0
// - circular orbit: argument of periapsis = 0
// - circular inclined: trueAnomaly stores argument of latitude
// - circular equatorial: trueAnomaly stores true longitude
// - eccentric equatorial: RAAN = 0 and argumentPeriapsis stores direction-aware
//   longitude of periapsis (inertial longitude is +omega prograde, -omega retrograde)
[[nodiscard]] std::optional<OrbitalElements> stateToOrbitalElements(
    const astronomy::StateVector& state,
    double muKm3PerSec2) noexcept;

[[nodiscard]] std::optional<astronomy::StateVector> orbitalElementsToState(
    const OrbitalElements& elements,
    double muKm3PerSec2) noexcept;

} // namespace solar::astrodynamics
