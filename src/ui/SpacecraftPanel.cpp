#include "ui/SpacecraftPanel.hpp"

#include "app/AppState.hpp"
#include "astrodynamics/Orbit.hpp"
#include "astronomy/BodyDatabase.hpp"
#include "astronomy/CelestialBody.hpp"
#include "renderer/Camera.hpp"
#include "renderer/Renderer.hpp"
#include "scene/SolarSystemScene.hpp"
#include "simulation/SimulationClock.hpp"
#include "simulation/Spacecraft.hpp"

#include <imgui.h>

#include <algorithm>
#include <numbers>

namespace solar::ui {

void SpacecraftPanel::render(
    app::AppState& state,
    std::unique_ptr<simulation::Spacecraft>& spacecraft,
    const astronomy::BodyDatabase& bodyDatabase,
    simulation::SimulationClock& clock,
    scene::SolarSystemScene& scene,
    renderer::Renderer& renderer)
{
    if (!state.panels.spacecraft) return;
    if (!ImGui::Begin("Spacecraft", &state.panels.spacecraft)) {
        ImGui::End();
        return;
    }
    ImGui::InputText("Name", m_name.data(), m_name.size());
    const auto& bodies = bodyDatabase.bodies();
    if (!bodies.empty()) {
        m_bodyIndex = std::clamp(m_bodyIndex, 0, static_cast<int>(bodies.size()) - 1);
        if (ImGui::BeginCombo("Central body", bodies[static_cast<std::size_t>(m_bodyIndex)].name().c_str())) {
            for (int index = 0; index < static_cast<int>(bodies.size()); ++index) {
                if (bodies[static_cast<std::size_t>(index)].gravitationalParameterKm3PerSec2() <= 0.0) continue;
                if (ImGui::Selectable(bodies[static_cast<std::size_t>(index)].name().c_str(), index == m_bodyIndex)) {
                    m_bodyIndex = index;
                }
            }
            ImGui::EndCombo();
        }
    }
    ImGui::Checkbox("State-vector input", &m_stateVectorMode);
    if (!m_stateVectorMode) {
        ImGui::InputDouble("Periapsis altitude (km)", &m_periapsisAltitudeKm, 10.0, 100.0, "%.3f");
        ImGui::InputDouble("Apoapsis altitude (km)", &m_apoapsisAltitudeKm, 10.0, 100.0, "%.3f");
        ImGui::InputDouble("Inclination (deg)", &m_inclinationDeg, 0.1, 1.0, "%.4f");
        ImGui::InputDouble("RAAN (deg)", &m_raanDeg, 0.1, 1.0, "%.4f");
        ImGui::InputDouble("Argument of periapsis (deg)", &m_argumentPeriapsisDeg, 0.1, 1.0, "%.4f");
        ImGui::InputDouble("True anomaly (deg)", &m_trueAnomalyDeg, 0.1, 1.0, "%.4f");
    } else {
        ImGui::InputScalarN("Position (km)", ImGuiDataType_Double, m_positionKm.data(), 3);
        ImGui::InputScalarN("Velocity (km/s)", ImGuiDataType_Double, m_velocityKmPerSec.data(), 3);
    }

    if (ImGui::Button("Create / Replace Orbit") && !bodies.empty()) {
        const auto& body = bodies[static_cast<std::size_t>(m_bodyIndex)];
        if (!m_stateVectorMode) {
            constexpr double radiansPerDegree = std::numbers::pi / 180.0;
            auto created = simulation::Spacecraft::fromApsisAltitudes(
                m_name.data(), body.name(), body.ephemerisId(), body.meanRadiusKm(),
                body.gravitationalParameterKm3PerSec2(), m_periapsisAltitudeKm,
                m_apoapsisAltitudeKm, m_inclinationDeg * radiansPerDegree,
                m_raanDeg * radiansPerDegree, m_argumentPeriapsisDeg * radiansPerDegree,
                m_trueAnomalyDeg * radiansPerDegree, clock.simulationTime());
            if (created) spacecraft = std::make_unique<simulation::Spacecraft>(std::move(*created));
        } else {
            try {
                const astronomy::StateVector vector{
                    {m_positionKm[0], m_positionKm[1], m_positionKm[2]},
                    {m_velocityKmPerSec[0], m_velocityKmPerSec[1], m_velocityKmPerSec[2]}};
                spacecraft = std::make_unique<simulation::Spacecraft>(
                    m_name.data(), body.name(), body.ephemerisId(), body.meanRadiusKm(),
                    astrodynamics::Orbit{vector, body.gravitationalParameterKm3PerSec2(),
                                        clock.simulationTime()});
            } catch (...) {
                spacecraft.reset();
            }
        }
        m_status = spacecraft ? "Orbit created at current simulation epoch"
                              : "Invalid orbit parameters";
    }

    if (spacecraft) {
        ImGui::SeparatorText("Active spacecraft");
        ImGui::Text("%s around %s", spacecraft->name().c_str(),
                    spacecraft->centralBodyName().c_str());
        if (ImGui::Button("Focus Spacecraft")) {
            scene.selectBody(bodyDatabase.find(spacecraft->centralBodyName())->naifId());
            scene.setReferenceOrigin(astronomy::OriginMode::selectedBody);
            scene.setDistanceScale(10'000.0);
            scene.followSelected = false;
            state.followSpacecraft = true;
            const glm::vec3 position = scene.renderTransform().toRenderPosition(
                spacecraft->current().relativeState.positionKm);
            renderer.camera().focus(position, 1.2F);
        }
        ImGui::SameLine();
        ImGui::Checkbox("Follow##spacecraft", &state.followSpacecraft);
    }
    if (!m_status.empty()) ImGui::TextDisabled("%s", m_status.c_str());
    ImGui::End();
}

} // namespace solar::ui
