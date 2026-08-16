#pragma once

#include "rocket/Stage.hpp"

#include <string>
#include <vector>

namespace solar::rocket {

// Stage order is chronological: stages[0] is the first stage to burn.
struct Vehicle final {
    std::string name{"Untitled Vehicle"};
    double payloadMassKg{};
    std::vector<Stage> stages;

    [[nodiscard]] double dryMassKg() const noexcept;
    [[nodiscard]] double propellantMassKg() const noexcept;
    [[nodiscard]] double liftoffMassKg() const noexcept;
    [[nodiscard]] double payloadFraction() const noexcept;
};

[[nodiscard]] ValidationResult validateVehicle(const Vehicle& vehicle);

} // namespace solar::rocket
