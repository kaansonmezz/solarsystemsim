#include "ui/ManeuverPanel.hpp"

#include "app/AppState.hpp"
#include "astrodynamics/Hohmann.hpp"
#include "astrodynamics/Maneuver.hpp"
#include "astrodynamics/PlaneChange.hpp"
#include "simulation/SimulationClock.hpp"
#include "simulation/Spacecraft.hpp"

#include <imgui.h>
#include <glm/geometric.hpp>

#include <cmath>
#include <numbers>

namespace solar::ui {
namespace {

void showPreviewResult(const simulation::Spacecraft& spacecraft)
{
    if (!spacecraft.previewOrbit()) return;
    const auto elements = spacecraft.previewOrbit()->elements();
    if (!elements) return;
    ImGui::SeparatorText("Preview result");
    ImGui::Text("Periapsis altitude: %.3f km",
                elements->periapsisRadiusKm() - spacecraft.centralBodyRadiusKm());
    if (const auto apoapsis = elements->apoapsisRadiusKm()) {
        ImGui::Text("Apoapsis altitude: %.3f km", *apoapsis - spacecraft.centralBodyRadiusKm());
    } else {
        ImGui::TextUnformatted("Apoapsis: N/A");
    }
    ImGui::Text("Eccentricity: %.9f", elements->eccentricity);
    ImGui::Text("Inclination: %.6f deg",
                elements->inclinationRad * 180.0 / std::numbers::pi);
    if (const auto period = elements->periodSeconds(spacecraft.orbit().muKm3PerSec2())) {
        ImGui::Text("Period: %.2f s", *period);
    } else {
        ImGui::TextUnformatted("Period: N/A");
    }
}

} // namespace

void ManeuverPanel::render(
    app::AppState& state,
    simulation::Spacecraft* spacecraft,
    simulation::SimulationClock& clock)
{
    if (!state.panels.maneuver) return;
    if (!ImGui::Begin("Maneuver", &state.panels.maneuver)) {
        ImGui::End();
        return;
    }
    if (spacecraft == nullptr) {
        ImGui::TextDisabled("Create a spacecraft to use the maneuver sandbox.");
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Impulsive burn at current epoch");
    ImGui::InputDouble("Tangential / prograde (m/s)", &m_tangentialMps, 1.0, 10.0, "%.3f");
    ImGui::InputDouble("Normal (m/s)", &m_normalMps, 1.0, 10.0, "%.3f");
    ImGui::InputDouble("Radial out (m/s)", &m_radialMps, 1.0, 10.0, "%.3f");
    const astrodynamics::ImpulsiveManeuver maneuver{
        m_tangentialMps / 1000.0, m_normalMps / 1000.0, m_radialMps / 1000.0};
    ImGui::Text("Total delta-v: %.3f m/s", maneuver.magnitudeKmPerSec() * 1000.0);
    if (ImGui::Button("Preview")) {
        static_cast<void>(spacecraft->previewManeuver(maneuver, clock.simulationTime()));
    }
    ImGui::SameLine();
    if (ImGui::Button("Apply")) static_cast<void>(spacecraft->applyPreview());
    ImGui::SameLine();
    if (ImGui::Button("Reset")) spacecraft->resetPreview();
    showPreviewResult(*spacecraft);

    ImGui::SeparatorText("Apsis helpers");
    if (ImGui::Button("Circularize Here")) {
        if (const auto delta = astrodynamics::circularizationDeltaVelocityKmPerSec(
                spacecraft->current().relativeState, spacecraft->orbit().muKm3PerSec2())) {
            m_tangentialMps = *delta * 1000.0;
        }
    }
    ImGui::InputDouble("Target opposite apsis altitude (km)",
                       &m_targetOppositeApsisAltitudeKm, 10.0, 100.0, "%.3f");
    if (ImGui::Button("Calculate Apsis Burn")) {
        const double radius = spacecraft->centralBodyRadiusKm() + m_targetOppositeApsisAltitudeKm;
        if (const auto delta = astrodynamics::apsidalTargetDeltaVelocityKmPerSec(
                spacecraft->current().relativeState, radius,
                spacecraft->orbit().muKm3PerSec2())) {
            m_tangentialMps = *delta * 1000.0;
        }
    }

    ImGui::SeparatorText("Hohmann from current circular orbit");
    ImGui::Text("Actual initial radius: %.3f km",
                glm::length(spacecraft->current().relativeState.positionKm));
    ImGui::InputDouble("Target altitude (km)##hohmann", &m_hohmannTargetAltitudeKm,
                       10.0, 100.0, "%.3f");
    const auto hohmann = astrodynamics::calculateHohmannFromCurrentCircularState(
        spacecraft->current().relativeState,
        spacecraft->centralBodyRadiusKm() + m_hohmannTargetAltitudeKm,
        spacecraft->orbit().muKm3PerSec2());
    if (hohmann) {
        ImGui::Text("Burn 1: %+.3f m/s", hohmann->transfer.firstBurnKmPerSec * 1000.0);
        ImGui::Text("Burn 2: %+.3f m/s", hohmann->transfer.secondBurnKmPerSec * 1000.0);
        ImGui::Text("Total: %.3f m/s", hohmann->transfer.totalDeltaVelocityKmPerSec * 1000.0);
        ImGui::Text("Transfer time: %.2f min", hohmann->transfer.transferTimeSeconds / 60.0);
        if (ImGui::Button("Preview Hohmann Burn 1")) {
            static_cast<void>(spacecraft->previewManeuver(
                {hohmann->transfer.firstBurnKmPerSec, 0.0, 0.0}, clock.simulationTime()));
        }
    } else {
        ImGui::TextDisabled("Unavailable: current orbit must be circular (e <= 1e-6).");
    }

    ImGui::SeparatorText("Ideal plane change");
    const auto elements = spacecraft->orbit().elements();
    const double currentInclinationDeg = elements
        ? elements->inclinationRad * 180.0 / std::numbers::pi : 0.0;
    ImGui::Text("Current inclination: %.6f deg", currentInclinationDeg);
    ImGui::InputDouble("Target inclination (deg)", &m_targetInclinationDeg,
                       0.1, 1.0, "%.4f");
    const double inclinationChange = (m_targetInclinationDeg - currentInclinationDeg) *
                                     std::numbers::pi / 180.0;
    if (const auto planeDelta = astrodynamics::planeChangeDeltaVelocityKmPerSec(
            glm::length(spacecraft->current().relativeState.velocityKmPerSec), inclinationChange)) {
        ImGui::Text("Required delta-v: %.3f m/s", *planeDelta * 1000.0);
    }
    ImGui::TextDisabled("Pure instantaneous plane change at unchanged speed magnitude.");
    ImGui::End();
}

} // namespace solar::ui
