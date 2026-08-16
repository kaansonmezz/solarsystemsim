#include "rocket/MassFlow.hpp"

#include "rocket/Units.hpp"

#include <cmath>

namespace solar::rocket {

std::optional<double> massFlowKgPerSec(const double thrustN, const double ispSeconds) noexcept
{
    if (!std::isfinite(thrustN) || thrustN < 0.0 ||
        !std::isfinite(ispSeconds) || !(ispSeconds > 0.0)) return std::nullopt;
    const double result = thrustN / (ispSeconds * kStandardGravityMetersPerSec2);
    return std::isfinite(result) ? std::optional<double>{result} : std::nullopt;
}

std::optional<double> engineClusterThrustN(
    const Engine& engine,
    const int engineCount,
    const PerformanceEnvironment environment,
    const double throttle) noexcept
{
    if (engineCount <= 0 || !std::isfinite(throttle) ||
        throttle < engine.minimumThrottle || throttle > engine.maximumThrottle) {
        return std::nullopt;
    }
    const double perEngine = engine.thrustN(environment);
    if (!std::isfinite(perEngine) || perEngine < 0.0) return std::nullopt;
    const double result = perEngine * static_cast<double>(engineCount) * throttle;
    return std::isfinite(result) ? std::optional<double>{result} : std::nullopt;
}

std::optional<double> engineClusterMassFlowKgPerSec(
    const Engine& engine,
    const int engineCount,
    const PerformanceEnvironment environment,
    const double throttle) noexcept
{
    const auto thrust = engineClusterThrustN(engine, engineCount, environment, throttle);
    return thrust ? massFlowKgPerSec(*thrust, engine.ispSec(environment)) : std::nullopt;
}

} // namespace solar::rocket
