#pragma once

#include <optional>

namespace solar::rocket {

[[nodiscard]] std::optional<double> effectiveExhaustVelocityMetersPerSec(
    double ispSeconds) noexcept;
[[nodiscard]] std::optional<double> idealDeltaVMetersPerSec(
    double initialMassKg, double finalMassKg, double ispSeconds) noexcept;
[[nodiscard]] std::optional<double> requiredPropellantMassKg(
    double initialMassKg, double requestedDeltaVMetersPerSec, double ispSeconds) noexcept;
[[nodiscard]] std::optional<double> remainingIdealDeltaVMetersPerSec(
    double currentWetMassKg, double remainingPropellantMassKg, double ispSeconds) noexcept;

} // namespace solar::rocket
