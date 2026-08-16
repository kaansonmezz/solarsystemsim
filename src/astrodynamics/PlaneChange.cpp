#include "astrodynamics/PlaneChange.hpp"

#include <cmath>

namespace solar::astrodynamics {

std::optional<double> planeChangeDeltaVelocityKmPerSec(
    const double speed, const double inclinationChangeRad) noexcept
{
    if (!(speed >= 0.0) || !std::isfinite(speed) || !std::isfinite(inclinationChangeRad)) {
        return std::nullopt;
    }
    return 2.0 * speed * std::abs(std::sin(inclinationChangeRad / 2.0));
}

} // namespace solar::astrodynamics
