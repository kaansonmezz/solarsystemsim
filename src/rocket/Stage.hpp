#pragma once

#include "rocket/Engine.hpp"
#include "rocket/Propellant.hpp"

#include <string>

namespace solar::rocket {

// Dry mass explicitly includes the mounted engine cluster:
// structural + other dry + interstage + engine.mass * engineCount.
struct Stage final {
    std::string name{"Stage"};
    double structuralMassKg{};
    double otherDryMassKg{};
    double interstageMassKg{};
    Propellant propellant;
    Engine engine;
    int engineCount{1};
    double throttle{1.0};

    [[nodiscard]] double engineClusterMassKg() const noexcept;
    [[nodiscard]] double dryMassKg() const noexcept;
    [[nodiscard]] double wetMassKg() const noexcept;
    [[nodiscard]] double structuralCoefficient() const noexcept;
    [[nodiscard]] double propellantFraction() const noexcept;
};

[[nodiscard]] ValidationResult validateStage(const Stage& stage);

} // namespace solar::rocket
