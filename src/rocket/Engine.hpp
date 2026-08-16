#pragma once

#include "rocket/Validation.hpp"

#include <string>

namespace solar::rocket {

enum class PerformanceEnvironment {
    vacuum,
    seaLevel,
};

struct Engine final {
    std::string name{"Custom Engine"};
    std::string manufacturer;
    std::string description;
    double thrustVacuumN{};
    double thrustSeaLevelN{};
    double ispVacuumSec{};
    double ispSeaLevelSec{};
    double massKg{};
    double minimumThrottle{0.0};
    double maximumThrottle{1.0};

    [[nodiscard]] double thrustN(PerformanceEnvironment environment) const noexcept;
    [[nodiscard]] double ispSec(PerformanceEnvironment environment) const noexcept;
};

[[nodiscard]] ValidationResult validateEngine(const Engine& engine);
[[nodiscard]] const char* environmentName(PerformanceEnvironment environment) noexcept;

} // namespace solar::rocket
