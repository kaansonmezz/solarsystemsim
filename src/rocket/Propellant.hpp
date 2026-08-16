#pragma once

#include "rocket/Validation.hpp"

#include <string>

namespace solar::rocket {

enum class PropellantCategory {
    liquid,
    solid,
    monopropellant,
    electric,
    generic,
};

struct Propellant final {
    std::string name{"Generic Propellant"};
    PropellantCategory category{PropellantCategory::generic};
    double massKg{};
};

[[nodiscard]] ValidationResult validatePropellant(const Propellant& propellant);
[[nodiscard]] const char* propellantCategoryName(PropellantCategory category) noexcept;

} // namespace solar::rocket
