#pragma once

#include "astronomy/StateVector.hpp"

#include <optional>

namespace solar::astrodynamics {

// Ideal point-mass two-body propagation. Near-parabolic states are rejected explicitly.
[[nodiscard]] std::optional<astronomy::StateVector> propagateTwoBody(
    const astronomy::StateVector& stateAtEpoch,
    double muKm3PerSec2,
    double deltaTimeSeconds) noexcept;

} // namespace solar::astrodynamics
