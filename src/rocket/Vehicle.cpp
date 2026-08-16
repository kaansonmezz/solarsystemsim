#include "rocket/Vehicle.hpp"

#include <cmath>
#include <string>

namespace solar::rocket {

double Vehicle::dryMassKg() const noexcept
{
    double total = payloadMassKg;
    for (const auto& stage : stages) total += stage.dryMassKg();
    return total;
}

double Vehicle::propellantMassKg() const noexcept
{
    double total = 0.0;
    for (const auto& stage : stages) total += stage.propellant.massKg;
    return total;
}

double Vehicle::liftoffMassKg() const noexcept
{
    double total = payloadMassKg;
    for (const auto& stage : stages) total += stage.wetMassKg();
    return total;
}

double Vehicle::payloadFraction() const noexcept
{
    const double liftoff = liftoffMassKg();
    return liftoff > 0.0 ? payloadMassKg / liftoff : 0.0;
}

ValidationResult validateVehicle(const Vehicle& vehicle)
{
    ValidationResult result;
    if (vehicle.name.empty()) result.add("Vehicle name must not be empty");
    if (!std::isfinite(vehicle.payloadMassKg) || vehicle.payloadMassKg < 0.0) {
        result.add("Payload mass must be finite and non-negative");
    }
    for (std::size_t index = 0; index < vehicle.stages.size(); ++index) {
        const auto stageResult = validateStage(vehicle.stages[index]);
        for (const auto& error : stageResult.errors) {
            result.add("Stage " + std::to_string(index + 1) + ": " + error);
        }
    }
    return result;
}

} // namespace solar::rocket
