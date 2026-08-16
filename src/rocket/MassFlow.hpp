#pragma once

#include "rocket/Engine.hpp"

#include <optional>

namespace solar::rocket {

[[nodiscard]] std::optional<double> massFlowKgPerSec(
    double thrustN, double ispSeconds) noexcept;
[[nodiscard]] std::optional<double> engineClusterThrustN(
    const Engine& engine, int engineCount, PerformanceEnvironment environment,
    double throttle = 1.0) noexcept;
[[nodiscard]] std::optional<double> engineClusterMassFlowKgPerSec(
    const Engine& engine, int engineCount, PerformanceEnvironment environment,
    double throttle = 1.0) noexcept;

} // namespace solar::rocket
