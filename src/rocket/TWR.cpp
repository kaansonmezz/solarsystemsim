#include "rocket/TWR.hpp"

#include "astronomy/CelestialBody.hpp"

#include <cmath>

namespace solar::rocket {

std::optional<double> thrustToWeightRatio(
    const double thrustN, const double massKg, const double gravityMetersPerSec2) noexcept
{
    if (!std::isfinite(thrustN) || thrustN < 0.0 ||
        !std::isfinite(massKg) || !(massKg > 0.0) ||
        !std::isfinite(gravityMetersPerSec2) || !(gravityMetersPerSec2 > 0.0)) {
        return std::nullopt;
    }
    const double result = thrustN / (massKg * gravityMetersPerSec2);
    return std::isfinite(result) ? std::optional<double>{result} : std::nullopt;
}

std::optional<double> thrustToWeightRatio(
    const double thrustN, const double massKg, const astronomy::CelestialBody& body) noexcept
{
    return thrustToWeightRatio(thrustN, massKg, body.surfaceGravityMetersPerSec2());
}

} // namespace solar::rocket
