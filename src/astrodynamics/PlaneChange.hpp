#pragma once

#include <optional>

namespace solar::astrodynamics {

[[nodiscard]] std::optional<double> planeChangeDeltaVelocityKmPerSec(
    double speedKmPerSec, double inclinationChangeRad) noexcept;

} // namespace solar::astrodynamics
