#include "rocket/Stage.hpp"

#include <cmath>
#include <limits>

namespace solar::rocket {

double Stage::engineClusterMassKg() const noexcept
{
    return engineCount > 0 ? engine.massKg * static_cast<double>(engineCount) : 0.0;
}

double Stage::dryMassKg() const noexcept
{
    return structuralMassKg + otherDryMassKg + interstageMassKg + engineClusterMassKg();
}

double Stage::wetMassKg() const noexcept { return dryMassKg() + propellant.massKg; }

double Stage::structuralCoefficient() const noexcept
{
    const double wet = wetMassKg();
    return wet > 0.0 ? dryMassKg() / wet : 0.0;
}

double Stage::propellantFraction() const noexcept
{
    const double wet = wetMassKg();
    return wet > 0.0 ? propellant.massKg / wet : 0.0;
}

ValidationResult validateStage(const Stage& stage)
{
    ValidationResult result;
    const auto requireMass = [&](const double value, const char* label) {
        if (!std::isfinite(value) || value < 0.0) {
            result.add(std::string{label} + " must be finite and non-negative");
        }
    };
    if (stage.name.empty()) result.add("Stage name must not be empty");
    requireMass(stage.structuralMassKg, "Structural mass");
    requireMass(stage.otherDryMassKg, "Other dry mass");
    requireMass(stage.interstageMassKg, "Interstage mass");
    const auto propellantValidation = validatePropellant(stage.propellant);
    result.errors.insert(result.errors.end(), propellantValidation.errors.begin(),
                         propellantValidation.errors.end());
    const auto engineValidation = validateEngine(stage.engine);
    result.errors.insert(result.errors.end(), engineValidation.errors.begin(),
                         engineValidation.errors.end());
    if (stage.engineCount <= 0) result.add("Engine count must be greater than zero");
    if (!std::isfinite(stage.throttle) || stage.throttle < stage.engine.minimumThrottle ||
        stage.throttle > stage.engine.maximumThrottle) {
        result.add("Stage throttle must be inside the engine throttle range");
    }
    return result;
}

} // namespace solar::rocket
