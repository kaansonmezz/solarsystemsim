#include "rocket/Propellant.hpp"

#include <cmath>

namespace solar::rocket {

ValidationResult validatePropellant(const Propellant& propellant)
{
    ValidationResult result;
    if (propellant.name.empty()) result.add("Propellant name must not be empty");
    if (!std::isfinite(propellant.massKg) || propellant.massKg < 0.0) {
        result.add("Propellant mass must be finite and non-negative");
    }
    return result;
}

const char* propellantCategoryName(const PropellantCategory category) noexcept
{
    switch (category) {
    case PropellantCategory::liquid: return "Liquid";
    case PropellantCategory::solid: return "Solid";
    case PropellantCategory::monopropellant: return "Monopropellant";
    case PropellantCategory::electric: return "Electric";
    case PropellantCategory::generic: return "Generic";
    }
    return "Generic";
}

} // namespace solar::rocket
