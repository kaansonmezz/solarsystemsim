#pragma once

#include <optional>

namespace solar::astronomy { class CelestialBody; }

namespace solar::rocket {

[[nodiscard]] std::optional<double> thrustToWeightRatio(
    double thrustN, double massKg, double gravityMetersPerSec2) noexcept;
[[nodiscard]] std::optional<double> thrustToWeightRatio(
    double thrustN, double massKg, const astronomy::CelestialBody& body) noexcept;

} // namespace solar::rocket
