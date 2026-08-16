#include "ui/RocketPanel.hpp"

#include "app/AppState.hpp"
#include "astronomy/BodyDatabase.hpp"
#include "core/FileSystem.hpp"
#include "rocket/EngineDatabase.hpp"
#include "rocket/RocketAnalysis.hpp"
#include "rocket/VehicleDatabase.hpp"

#include <imgui.h>
#include <implot.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <vector>

namespace solar::ui {
namespace {

[[nodiscard]] double optionalOrZero(const std::optional<double>& value) noexcept
{
    return value.value_or(0.0);
}

} // namespace

void RocketPanel::render(
    app::AppState& state,
    rocket::Vehicle& vehicle,
    const rocket::EngineDatabase& engineDatabase,
    const astronomy::BodyDatabase& bodyDatabase)
{
    const auto& bodies = bodyDatabase.bodies();
    if (!bodies.empty()) {
        m_bodyIndex = std::clamp(m_bodyIndex, 0, static_cast<int>(bodies.size()) - 1);
    }
    if (!vehicle.stages.empty()) {
        m_selectedStage = std::min(m_selectedStage, vehicle.stages.size() - 1);
    } else {
        m_selectedStage = 0;
    }
    const double gravity = referenceGravity(bodyDatabase);

    m_stagePanel.render(state, vehicle, m_selectedStage, m_environment, gravity);
    m_enginePanel.render(state, vehicle, m_selectedStage, engineDatabase, m_environment);

    if (!state.panels.rocket) return;
    if (!ImGui::Begin("ROCKET / VEHICLE", &state.panels.rocket)) {
        ImGui::End();
        return;
    }

    std::array<char, 128> name{};
    std::snprintf(name.data(), name.size(), "%s", vehicle.name.c_str());
    if (ImGui::InputText("Vehicle Name", name.data(), name.size())) vehicle.name = name.data();
    ImGui::InputDouble("Payload Mass (kg)", &vehicle.payloadMassKg, 10.0, 100.0, "%.3f");

    if (!bodies.empty() && ImGui::BeginCombo("Reference Body", bodies[static_cast<std::size_t>(m_bodyIndex)].name().c_str())) {
        for (std::size_t index = 0; index < bodies.size(); ++index) {
            const bool selected = static_cast<int>(index) == m_bodyIndex;
            if (ImGui::Selectable(bodies[index].name().c_str(), selected)) {
                m_bodyIndex = static_cast<int>(index);
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    int environmentIndex = m_environment == rocket::PerformanceEnvironment::vacuum ? 0 : 1;
    constexpr const char* environmentNames[]{"Vacuum", "Sea Level"};
    if (ImGui::Combo("Reference Performance", &environmentIndex, environmentNames, 2)) {
        m_environment = environmentIndex == 0 ? rocket::PerformanceEnvironment::vacuum
                                               : rocket::PerformanceEnvironment::seaLevel;
    }
    const double selectedGravity = referenceGravity(bodyDatabase);

    if (ImGui::Button("Add Stage")) addStage(vehicle, engineDatabase);
    ImGui::SameLine();
    if (ImGui::Button("Remove Stage") && !vehicle.stages.empty()) {
        vehicle.stages.erase(vehicle.stages.begin() + static_cast<std::ptrdiff_t>(m_selectedStage));
        if (!vehicle.stages.empty()) m_selectedStage = std::min(m_selectedStage, vehicle.stages.size() - 1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Move Stage Up") && m_selectedStage > 0 && !vehicle.stages.empty()) {
        std::swap(vehicle.stages[m_selectedStage], vehicle.stages[m_selectedStage - 1]);
        --m_selectedStage;
    }
    ImGui::SameLine();
    if (ImGui::Button("Move Stage Down") && m_selectedStage + 1 < vehicle.stages.size()) {
        std::swap(vehicle.stages[m_selectedStage], vehicle.stages[m_selectedStage + 1]);
        ++m_selectedStage;
    }

    if (!m_pathInitialized) {
        const auto path = core::FileSystem::config("vehicles/user_vehicle.json").string();
        std::snprintf(m_vehiclePath.data(), m_vehiclePath.size(), "%s", path.c_str());
        m_pathInitialized = true;
    }
    ImGui::InputText("Vehicle JSON", m_vehiclePath.data(), m_vehiclePath.size());
    if (ImGui::Button("Save Vehicle")) {
        const auto saved = rocket::saveVehicle(vehicle, std::filesystem::path{m_vehiclePath.data()});
        m_status = saved.valid() ? "Vehicle saved" : saved.errors.front();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Vehicle")) {
        auto loaded = rocket::loadVehicle(std::filesystem::path{m_vehiclePath.data()});
        if (loaded.valid()) {
            vehicle = std::move(*loaded.vehicle);
            m_selectedStage = 0;
            m_status = "Vehicle loaded";
        } else {
            m_status = loaded.errors.empty() ? "Vehicle load failed" : loaded.errors.front();
        }
    }
    if (!m_status.empty()) ImGui::TextDisabled("%s", m_status.c_str());

    const auto analysis = rocket::analyzeVehicle(vehicle, m_environment, selectedGravity);
    const auto vacuumAnalysis = rocket::analyzeVehicle(
        vehicle, rocket::PerformanceEnvironment::vacuum, selectedGravity);
    const auto seaLevelAnalysis = rocket::analyzeVehicle(
        vehicle, rocket::PerformanceEnvironment::seaLevel, selectedGravity);

    ImGui::SeparatorText("Vehicle Summary");
    ImGui::Text("Stage Count: %zu", vehicle.stages.size());
    ImGui::Text("Liftoff Mass: %.3f kg", analysis.liftoffMassKg);
    ImGui::Text("Dry Mass incl. payload: %.3f kg", analysis.dryMassKg);
    ImGui::Text("Propellant Mass: %.3f kg", analysis.propellantMassKg);
    ImGui::Text("Payload: %.3f kg (%.3f%%)", analysis.payloadMassKg,
                analysis.payloadFraction * 100.0);
    ImGui::Text("Ideal Delta-v using Vacuum Isp: %.3f m/s",
                vacuumAnalysis.totalIdealDeltaVMetersPerSec);
    ImGui::Text("Ideal Delta-v using Sea-Level Isp: %.3f m/s",
                seaLevelAnalysis.totalIdealDeltaVMetersPerSec);
    ImGui::Text("Reference Delta-v (%s): %.3f m/s", rocket::environmentName(m_environment),
                analysis.totalIdealDeltaVMetersPerSec);
    if (!bodies.empty()) {
        ImGui::Text("Reference Gravity (%s): %.6f m/s^2",
                    bodies[static_cast<std::size_t>(m_bodyIndex)].name().c_str(), selectedGravity);
    }

    renderStageTable(vehicle, analysis);
    renderCharts(vehicle, analysis);

    ImGui::SeparatorText("Engineering Warnings");
    if (analysis.issues.empty()) {
        ImGui::TextDisabled("No warnings.");
    } else {
        for (const auto& issue : analysis.issues) {
            const ImVec4 color = issue.severity == rocket::AnalysisIssueSeverity::error
                ? ImVec4{1.0F, 0.35F, 0.25F, 1.0F} : ImVec4{1.0F, 0.75F, 0.25F, 1.0F};
            if (issue.stageIndex) {
                ImGui::TextColored(color, "Stage %zu: %s", *issue.stageIndex + 1,
                                   issue.message.c_str());
            } else {
                ImGui::TextColored(color, "%s", issue.message.c_str());
            }
        }
    }
    ImGui::TextDisabled("Ideal reference only: no drag, gravity/steering losses, residuals, or finite-burn trajectory propagation.");
    ImGui::End();
}

double RocketPanel::referenceGravity(const astronomy::BodyDatabase& bodyDatabase) const
{
    const auto& bodies = bodyDatabase.bodies();
    if (bodies.empty()) return 0.0;
    const int index = std::clamp(m_bodyIndex, 0, static_cast<int>(bodies.size()) - 1);
    return bodies[static_cast<std::size_t>(index)].surfaceGravityMetersPerSec2();
}

void RocketPanel::addStage(
    rocket::Vehicle& vehicle, const rocket::EngineDatabase& engineDatabase)
{
    rocket::Stage stage;
    stage.name = "Stage " + std::to_string(vehicle.stages.size() + 1);
    stage.structuralMassKg = 1'000.0;
    stage.propellant.massKg = 5'000.0;
    if (!engineDatabase.engines().empty()) {
        stage.engine = engineDatabase.engines().front();
    } else {
        stage.engine = {"Custom Sample Engine", "", "User-editable fallback",
                        500'000.0, 450'000.0, 320.0, 290.0, 500.0, 0.0, 1.0};
    }
    vehicle.stages.push_back(std::move(stage));
    m_selectedStage = vehicle.stages.size() - 1;
}

void RocketPanel::renderStageTable(
    const rocket::Vehicle& vehicle, const rocket::VehicleAnalysis& analysis)
{
    ImGui::SeparatorText("Stage Sequence — row 1 burns first");
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX;
    if (!ImGui::BeginTable("RocketStageTable", 7, flags)) return;
    ImGui::TableSetupColumn("Stage");
    ImGui::TableSetupColumn("Wet kg");
    ImGui::TableSetupColumn("Propellant kg");
    ImGui::TableSetupColumn("Engines");
    ImGui::TableSetupColumn("Delta-v m/s");
    ImGui::TableSetupColumn("TWR ignition/final");
    ImGui::TableSetupColumn("Burn s");
    ImGui::TableHeadersRow();
    for (std::size_t index = 0; index < analysis.stages.size(); ++index) {
        const auto& stage = analysis.stages[index];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        const std::string label = std::to_string(index + 1) + " — " + stage.stageName;
        if (ImGui::Selectable(label.c_str(), index == m_selectedStage,
                              ImGuiSelectableFlags_SpanAllColumns)) m_selectedStage = index;
        ImGui::TableSetColumnIndex(1); ImGui::Text("%.0f", stage.wetMassKg);
        ImGui::TableSetColumnIndex(2); ImGui::Text("%.0f", stage.propellantMassKg);
        ImGui::TableSetColumnIndex(3); ImGui::Text("%d", vehicle.stages[index].engineCount);
        ImGui::TableSetColumnIndex(4); ImGui::Text("%.1f", optionalOrZero(stage.deltaVMetersPerSec));
        ImGui::TableSetColumnIndex(5); ImGui::Text("%.2f / %.2f",
            optionalOrZero(stage.initialTwr), optionalOrZero(stage.finalTwr));
        ImGui::TableSetColumnIndex(6); ImGui::Text("%.1f", optionalOrZero(stage.burnTimeSeconds));
    }
    ImGui::EndTable();
}

void RocketPanel::renderCharts(
    const rocket::Vehicle& vehicle, const rocket::VehicleAnalysis& analysis)
{
    if (analysis.stages.empty()) return;
    std::vector<double> deltaV;
    std::vector<double> dry;
    std::vector<double> propellant;
    std::vector<double> ticks;
    std::vector<std::string> labels;
    std::vector<const char*> labelPointers;
    const std::size_t count = analysis.stages.size();
    deltaV.reserve(count);
    dry.reserve(count + 1);
    propellant.reserve(count + 1);
    ticks.reserve(count + 1);
    labels.reserve(count + 1);
    dry.push_back(vehicle.payloadMassKg);
    propellant.push_back(0.0);
    ticks.push_back(0.0);
    labels.emplace_back("Payload");
    for (std::size_t index = 0; index < count; ++index) {
        deltaV.push_back(optionalOrZero(analysis.stages[index].deltaVMetersPerSec));
        dry.push_back(analysis.stages[index].dryMassKg);
        propellant.push_back(analysis.stages[index].propellantMassKg);
        ticks.push_back(static_cast<double>(index + 1));
        labels.push_back("S" + std::to_string(index + 1));
    }
    labelPointers.reserve(labels.size());
    for (const auto& label : labels) labelPointers.push_back(label.c_str());

    if (ImPlot::BeginPlot("Stage Delta-v Breakdown", {-1.0F, 190.0F}, ImPlotFlags_NoMouseText)) {
        ImPlot::SetupAxes("Stage (first burn at 1)", "Ideal Delta-v (m/s)",
                         ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::PlotBars("Delta-v", deltaV.data(), static_cast<int>(deltaV.size()), 0.65, 1.0);
        ImPlot::EndPlot();
    }
    if (ImPlot::BeginPlot("Mass Breakdown", {-1.0F, 190.0F}, ImPlotFlags_NoMouseText)) {
        ImPlot::SetupAxes("Payload / Stage", "Mass (kg)",
                         ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisTicks(ImAxis_X1, ticks.data(), static_cast<int>(ticks.size()),
                               labelPointers.data());
        constexpr const char* series[]{"Dry / Payload", "Propellant"};
        std::vector<double> grouped;
        grouped.reserve(dry.size() * 2);
        grouped.insert(grouped.end(), dry.begin(), dry.end());
        grouped.insert(grouped.end(), propellant.begin(), propellant.end());
        ImPlot::PlotBarGroups(series, grouped.data(), 2, static_cast<int>(dry.size()), 0.75, 0.0,
                              ImPlotBarGroupsFlags_Stacked);
        ImPlot::EndPlot();
    }
}

} // namespace solar::ui
