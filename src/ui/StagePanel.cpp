#include "ui/StagePanel.hpp"

#include "app/AppState.hpp"
#include "rocket/RocketAnalysis.hpp"
#include "rocket/Vehicle.hpp"

#include <imgui.h>

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace solar::ui {
namespace {

void showOptional(const char* label, const std::optional<double>& value, const char* unit = "")
{
    if (value) ImGui::Text("%s: %.3f %s", label, *value, unit);
    else ImGui::TextDisabled("%s: unavailable", label);
}

} // namespace

void StagePanel::render(
    app::AppState& state,
    rocket::Vehicle& vehicle,
    const std::size_t selectedStage,
    const rocket::PerformanceEnvironment environment,
    const double referenceGravityMetersPerSec2)
{
    if (!state.panels.stage) return;
    if (!ImGui::Begin("Rocket Stage", &state.panels.stage)) {
        ImGui::End();
        return;
    }
    if (vehicle.stages.empty() || selectedStage >= vehicle.stages.size()) {
        ImGui::TextDisabled("Add and select a stage in ROCKET / VEHICLE.");
        ImGui::End();
        return;
    }

    auto& stage = vehicle.stages[selectedStage];
    ImGui::Text("Stage %zu — %s burn", selectedStage + 1,
                selectedStage == 0 ? "first" : "subsequent");
    std::array<char, 128> name{};
    std::snprintf(name.data(), name.size(), "%s", stage.name.c_str());
    if (ImGui::InputText("Stage Name", name.data(), name.size())) stage.name = name.data();
    ImGui::InputDouble("Structural Mass (kg)", &stage.structuralMassKg, 10.0, 100.0, "%.3f");
    ImGui::InputDouble("Other Dry Mass (kg)", &stage.otherDryMassKg, 10.0, 100.0, "%.3f");
    ImGui::InputDouble("Interstage Mass (kg)", &stage.interstageMassKg, 10.0, 100.0, "%.3f");
    ImGui::InputDouble("Propellant Mass (kg)", &stage.propellant.massKg, 10.0, 100.0, "%.3f");
    ImGui::InputInt("Engine Count", &stage.engineCount, 1, 4);
    double throttlePercent = stage.throttle * 100.0;
    const bool validThrottleRange = std::isfinite(stage.engine.minimumThrottle) &&
        std::isfinite(stage.engine.maximumThrottle) && stage.engine.minimumThrottle >= 0.0 &&
        stage.engine.maximumThrottle <= 1.0 &&
        stage.engine.minimumThrottle <= stage.engine.maximumThrottle;
    const double minimumThrottlePercent = validThrottleRange
        ? stage.engine.minimumThrottle * 100.0 : 0.0;
    const double maximumThrottlePercent = validThrottleRange
        ? stage.engine.maximumThrottle * 100.0 : 100.0;
    if (!std::isfinite(throttlePercent)) throttlePercent = maximumThrottlePercent;
    throttlePercent = std::clamp(
        throttlePercent, minimumThrottlePercent, maximumThrottlePercent);
    if (ImGui::SliderScalar("Throttle (%)", ImGuiDataType_Double, &throttlePercent,
                            &minimumThrottlePercent, &maximumThrottlePercent, "%.1f")) {
        stage.throttle = throttlePercent / 100.0;
    }

    const auto analysis = rocket::analyzeVehicle(vehicle, environment, referenceGravityMetersPerSec2);
    if (selectedStage < analysis.stages.size()) {
        const auto& calculated = analysis.stages[selectedStage];
        ImGui::SeparatorText("Derived — read only");
        ImGui::Text("Performance: %s", rocket::environmentName(environment));
        ImGui::Text("Dry Mass: %.3f kg", calculated.dryMassKg);
        ImGui::Text("Wet Mass: %.3f kg", calculated.wetMassKg);
        ImGui::Text("Stack Ignition Mass: %.3f kg", calculated.ignitionMassKg);
        ImGui::Text("Burnout Mass (before separation): %.3f kg", calculated.burnoutMassKg);
        ImGui::Text("Post-Separation Mass: %.3f kg", calculated.postSeparationMassKg);
        showOptional("Cluster Thrust", calculated.thrustN, "N");
        showOptional("Mass Flow", calculated.massFlowKgPerSec, "kg/s");
        showOptional("Ideal Delta-v", calculated.deltaVMetersPerSec, "m/s");
        showOptional("Initial TWR", calculated.initialTwr);
        showOptional("Final TWR", calculated.finalTwr);
        showOptional("Burn Time Estimate", calculated.burnTimeSeconds, "s");
        showOptional("Burn Mass Ratio m0/mf", calculated.burnMassRatio);
        ImGui::Text("Structural Coefficient: %.5f", calculated.structuralCoefficient);
        ImGui::Text("Propellant Fraction: %.5f", calculated.propellantFraction);
    } else {
        ImGui::SeparatorText("Validation");
        ImGui::TextColored({1.0F, 0.45F, 0.30F, 1.0F},
                           "Correct invalid inputs to calculate this stage.");
    }
    ImGui::End();
}

} // namespace solar::ui
