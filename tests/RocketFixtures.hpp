#pragma once

#include "rocket/Engine.hpp"
#include "rocket/Stage.hpp"
#include "rocket/Vehicle.hpp"

namespace solar::test {

[[nodiscard]] inline rocket::Engine testEngine()
{
    return {
        "Analytical Test Engine", "SOLAR Tests", "Not a real engine",
        1'000'000.0, 900'000.0, 320.0, 300.0, 1'000.0, 0.0, 1.0};
}

[[nodiscard]] inline rocket::Stage testStage(
    const char* name, const double structuralKg, const double propellantKg,
    const int engineCount = 1)
{
    rocket::Stage stage;
    stage.name = name;
    stage.structuralMassKg = structuralKg;
    stage.propellant.massKg = propellantKg;
    stage.engine = testEngine();
    stage.engineCount = engineCount;
    return stage;
}

[[nodiscard]] inline rocket::Vehicle twoStageVehicle(const double payloadKg = 5'000.0)
{
    rocket::Vehicle vehicle;
    vehicle.name = "Analytical Two Stage";
    vehicle.payloadMassKg = payloadKg;
    vehicle.stages.push_back(testStage("Stage 1", 20'000.0, 100'000.0, 2));
    vehicle.stages.push_back(testStage("Stage 2", 5'000.0, 20'000.0, 1));
    return vehicle;
}

} // namespace solar::test
