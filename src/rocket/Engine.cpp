#include "rocket/Engine.hpp"

#include <cmath>

namespace solar::rocket {

double Engine::thrustN(const PerformanceEnvironment environment) const noexcept
{
    return environment == PerformanceEnvironment::vacuum ? thrustVacuumN : thrustSeaLevelN;
}

double Engine::ispSec(const PerformanceEnvironment environment) const noexcept
{
    return environment == PerformanceEnvironment::vacuum ? ispVacuumSec : ispSeaLevelSec;
}

ValidationResult validateEngine(const Engine& engine)
{
    ValidationResult result;
    const auto finite = [](const double value) { return std::isfinite(value); };
    if (engine.name.empty()) result.add("Engine name must not be empty");
    if (!finite(engine.thrustVacuumN) || engine.thrustVacuumN < 0.0) {
        result.add("Vacuum thrust must be finite and non-negative");
    }
    if (!finite(engine.thrustSeaLevelN) || engine.thrustSeaLevelN < 0.0) {
        result.add("Sea-level thrust must be finite and non-negative");
    }
    if (!finite(engine.ispVacuumSec) || engine.ispVacuumSec <= 0.0) {
        result.add("Vacuum specific impulse must be finite and positive");
    }
    if (!finite(engine.ispSeaLevelSec) || engine.ispSeaLevelSec <= 0.0) {
        result.add("Sea-level specific impulse must be finite and positive");
    }
    if (!finite(engine.massKg) || engine.massKg < 0.0) {
        result.add("Engine mass must be finite and non-negative");
    }
    if (!finite(engine.minimumThrottle) || !finite(engine.maximumThrottle) ||
        engine.minimumThrottle < 0.0 || engine.maximumThrottle > 1.0 ||
        engine.minimumThrottle > engine.maximumThrottle) {
        result.add("Throttle range must satisfy 0 <= minimum <= maximum <= 1");
    }
    return result;
}

const char* environmentName(const PerformanceEnvironment environment) noexcept
{
    return environment == PerformanceEnvironment::vacuum ? "Vacuum" : "Sea Level";
}

} // namespace solar::rocket
