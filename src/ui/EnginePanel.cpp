#include "ui/EnginePanel.hpp"

#include "app/AppState.hpp"
#include "rocket/DeltaV.hpp"
#include "rocket/EngineDatabase.hpp"
#include "rocket/MassFlow.hpp"
#include "rocket/Vehicle.hpp"

#include <imgui.h>

#include <array>
#include <cstdio>

namespace solar::ui {

void EnginePanel::render(
    app::AppState& state,
    rocket::Vehicle& vehicle,
    const std::size_t selectedStage,
    const rocket::EngineDatabase& engineDatabase,
    const rocket::PerformanceEnvironment environment)
{
    if (!state.panels.engine) return;
    if (!ImGui::Begin("Rocket Engine", &state.panels.engine)) {
        ImGui::End();
        return;
    }
    if (vehicle.stages.empty() || selectedStage >= vehicle.stages.size()) {
        ImGui::TextDisabled("Add and select a stage to configure its engine.");
        ImGui::End();
        return;
    }

    auto& stage = vehicle.stages[selectedStage];
    auto& engine = stage.engine;
    const auto& engines = engineDatabase.engines();
    if (!engines.empty() && ImGui::BeginCombo("Engine Database", engine.name.c_str())) {
        for (const auto& candidate : engines) {
            const bool selected = candidate.name == engine.name;
            if (ImGui::Selectable(candidate.name.c_str(), selected)) engine = candidate;
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::TextDisabled("Database entries are convenience presets; every field remains editable.");

    std::array<char, 128> name{};
    std::snprintf(name.data(), name.size(), "%s", engine.name.c_str());
    if (ImGui::InputText("Engine Name", name.data(), name.size())) engine.name = name.data();
    ImGui::Text("Manufacturer: %s", engine.manufacturer.empty() ? "Custom / unspecified"
                                                               : engine.manufacturer.c_str());
    if (!engine.description.empty()) ImGui::TextWrapped("%s", engine.description.c_str());
    ImGui::InputDouble("Vacuum Thrust (N)", &engine.thrustVacuumN, 100.0, 1'000.0, "%.3f");
    ImGui::InputDouble("Sea-Level Thrust (N)", &engine.thrustSeaLevelN, 100.0, 1'000.0, "%.3f");
    ImGui::InputDouble("Vacuum Isp (s)", &engine.ispVacuumSec, 1.0, 10.0, "%.3f");
    ImGui::InputDouble("Sea-Level Isp (s)", &engine.ispSeaLevelSec, 1.0, 10.0, "%.3f");
    ImGui::InputDouble("Engine Mass (kg)", &engine.massKg, 1.0, 10.0, "%.3f");
    ImGui::InputDouble("Minimum Throttle", &engine.minimumThrottle, 0.01, 0.1, "%.3f");
    ImGui::InputDouble("Maximum Throttle", &engine.maximumThrottle, 0.01, 0.1, "%.3f");
    ImGui::InputInt("Mounted Engine Count", &stage.engineCount, 1, 4);

    ImGui::SeparatorText("Derived — read only");
    ImGui::Text("Selected Performance: %s", rocket::environmentName(environment));
    const auto exhaust = rocket::effectiveExhaustVelocityMetersPerSec(engine.ispSec(environment));
    const auto thrust = rocket::engineClusterThrustN(
        engine, stage.engineCount, environment, stage.throttle);
    const auto flow = rocket::engineClusterMassFlowKgPerSec(
        engine, stage.engineCount, environment, stage.throttle);
    if (exhaust) ImGui::Text("Effective Exhaust Velocity: %.3f m/s", *exhaust);
    else ImGui::TextDisabled("Effective Exhaust Velocity: unavailable");
    if (thrust) ImGui::Text("Cluster Thrust: %.3f N", *thrust);
    else ImGui::TextDisabled("Cluster Thrust: unavailable");
    if (flow) ImGui::Text("Cluster Mass Flow: %.6f kg/s", *flow);
    else ImGui::TextDisabled("Cluster Mass Flow: unavailable");

    const auto validation = rocket::validateEngine(engine);
    for (const auto& error : validation.errors) {
        ImGui::TextColored({1.0F, 0.45F, 0.30F, 1.0F}, "%s", error.c_str());
    }
    ImGui::End();
}

} // namespace solar::ui
