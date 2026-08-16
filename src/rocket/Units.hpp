#pragma once

namespace solar::rocket {

// Rocket engineering is SI. Astrodynamics stores velocity in km/s, so every
// crossing of that boundary must use these named conversions.
inline constexpr double kStandardGravityMetersPerSec2 = 9.80665;
inline constexpr double kMetersPerKilometer = 1'000.0;

[[nodiscard]] constexpr double metersPerSecToKilometersPerSec(const double value) noexcept
{
    return value / kMetersPerKilometer;
}

[[nodiscard]] constexpr double kilometersPerSecToMetersPerSec(const double value) noexcept
{
    return value * kMetersPerKilometer;
}

} // namespace solar::rocket
