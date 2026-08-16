#include "rocket/DeltaV.hpp"

#include "rocket/Units.hpp"

#include <cmath>

namespace solar::rocket {

std::optional<double> effectiveExhaustVelocityMetersPerSec(const double ispSeconds) noexcept
{
    if (!std::isfinite(ispSeconds) || !(ispSeconds > 0.0)) return std::nullopt;
    const double value = ispSeconds * kStandardGravityMetersPerSec2;
    return std::isfinite(value) ? std::optional<double>{value} : std::nullopt;
}

std::optional<double> idealDeltaVMetersPerSec(
    const double initialMassKg, const double finalMassKg, const double ispSeconds) noexcept
{
    if (!std::isfinite(initialMassKg) || !std::isfinite(finalMassKg) ||
        !(initialMassKg > 0.0) || !(finalMassKg > 0.0) || finalMassKg > initialMassKg) {
        return std::nullopt;
    }
    if (initialMassKg == finalMassKg) return 0.0;
    const auto exhaustVelocity = effectiveExhaustVelocityMetersPerSec(ispSeconds);
    if (!exhaustVelocity) return std::nullopt;
    const double result = *exhaustVelocity * std::log(initialMassKg / finalMassKg);
    return std::isfinite(result) ? std::optional<double>{result} : std::nullopt;
}

std::optional<double> requiredPropellantMassKg(
    const double initialMassKg,
    const double requestedDeltaVMetersPerSec,
    const double ispSeconds) noexcept
{
    if (!std::isfinite(initialMassKg) || !(initialMassKg > 0.0) ||
        !std::isfinite(requestedDeltaVMetersPerSec) || requestedDeltaVMetersPerSec < 0.0) {
        return std::nullopt;
    }
    const auto exhaustVelocity = effectiveExhaustVelocityMetersPerSec(ispSeconds);
    if (!exhaustVelocity) return std::nullopt;
    const double finalMass = initialMassKg *
        std::exp(-requestedDeltaVMetersPerSec / *exhaustVelocity);
    const double result = initialMassKg - finalMass;
    return std::isfinite(result) ? std::optional<double>{result} : std::nullopt;
}

std::optional<double> remainingIdealDeltaVMetersPerSec(
    const double currentWetMassKg,
    const double remainingPropellantMassKg,
    const double ispSeconds) noexcept
{
    if (!std::isfinite(remainingPropellantMassKg) || remainingPropellantMassKg < 0.0 ||
        !(currentWetMassKg > remainingPropellantMassKg)) {
        return std::nullopt;
    }
    return idealDeltaVMetersPerSec(
        currentWetMassKg, currentWetMassKg - remainingPropellantMassKg, ispSeconds);
}

} // namespace solar::rocket
