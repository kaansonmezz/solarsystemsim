#include "rocket/Burn.hpp"

#include <cmath>

namespace solar::rocket {

std::optional<double> burnTimeSeconds(
    const double propellantMassKg, const double massFlowKgPerSec) noexcept
{
    if (!std::isfinite(propellantMassKg) || propellantMassKg < 0.0 ||
        !std::isfinite(massFlowKgPerSec) || massFlowKgPerSec < 0.0) {
        return std::nullopt;
    }
    if (propellantMassKg == 0.0) return 0.0;
    if (!(massFlowKgPerSec > 0.0)) return std::nullopt;
    const double result = propellantMassKg / massFlowKgPerSec;
    return std::isfinite(result) ? std::optional<double>{result} : std::nullopt;
}

} // namespace solar::rocket
