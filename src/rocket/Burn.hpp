#pragma once

#include <optional>

namespace solar::rocket {

[[nodiscard]] std::optional<double> burnTimeSeconds(
    double propellantMassKg, double massFlowKgPerSec) noexcept;

} // namespace solar::rocket
