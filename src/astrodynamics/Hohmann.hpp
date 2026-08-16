#pragma once

#include "astronomy/StateVector.hpp"

#include <optional>

namespace solar::astrodynamics {

struct HohmannTransfer final {
    double initialCircularVelocityKmPerSec{};
    double finalCircularVelocityKmPerSec{};
    double transferDepartureVelocityKmPerSec{};
    double transferArrivalVelocityKmPerSec{};
    double firstBurnKmPerSec{};
    double secondBurnKmPerSec{};
    double totalDeltaVelocityKmPerSec{};
    double transferTimeSeconds{};
    double transferSemiMajorAxisKm{};
};

struct CurrentOrbitHohmann final {
    HohmannTransfer transfer;
    double initialRadiusKm{};
    double targetRadiusKm{};
};

[[nodiscard]] std::optional<HohmannTransfer> calculateHohmannTransfer(
    double initialRadiusKm,
    double finalRadiusKm,
    double muKm3PerSec2) noexcept;

// Applies only to a genuinely circular current state; no unrelated synthetic
// initial radius may be substituted when this result is used for a spacecraft burn.
[[nodiscard]] std::optional<CurrentOrbitHohmann> calculateHohmannFromCurrentCircularState(
    const astronomy::StateVector& currentState,
    double targetRadiusKm,
    double muKm3PerSec2,
    double circularEccentricityTolerance = 1.0e-6) noexcept;

} // namespace solar::astrodynamics
