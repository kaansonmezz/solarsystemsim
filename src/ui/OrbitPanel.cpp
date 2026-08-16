#include "ui/OrbitPanel.hpp"

#include "app/AppState.hpp"
#include "astrodynamics/Angles.hpp"
#include "astrodynamics/OrbitalMechanics.hpp"
#include "simulation/Spacecraft.hpp"

#include <imgui.h>
#include <glm/geometric.hpp>

#include <cmath>
#include <numbers>

namespace solar::ui {
namespace {
constexpr double kDegreesPerRadian = 180.0 / std::numbers::pi;

void optionalDistance(const char* label, const std::optional<double>& value, const double bodyRadius)
{
    if (value) ImGui::Text("%s: %.3f km (alt %.3f km)", label, *value, *value - bodyRadius);
    else ImGui::Text("%s: N/A", label);
}

void duration(const char* label, const std::optional<double>& seconds)
{
    if (!seconds) {
        ImGui::Text("%s: N/A", label);
        return;
    }
    const int total = static_cast<int>(*seconds);
    ImGui::Text("%s: %d h %d min %d s", label, total / 3600, (total % 3600) / 60, total % 60);
}
} // namespace

void OrbitPanel::render(app::AppState& state, const simulation::Spacecraft* spacecraft) const
{
    if (!state.panels.orbit) return;
    if (!ImGui::Begin("Orbit", &state.panels.orbit)) {
        ImGui::End();
        return;
    }
    if (spacecraft == nullptr) {
        ImGui::TextDisabled("Create a spacecraft to inspect an orbit.");
        ImGui::End();
        return;
    }
    const auto elements = spacecraft->orbit().elements();
    if (!elements) {
        ImGui::TextUnformatted("Invalid / degenerate orbit");
        ImGui::End();
        return;
    }
    const auto& vector = spacecraft->current().relativeState;
    ImGui::Text("Central body: %s", spacecraft->centralBodyName().c_str());
    ImGui::Text("Orbit type: %s", astrodynamics::name(elements->type()).data());
    ImGui::SeparatorText("Classical elements");
    ImGui::Text("Semi-major axis: %.6f km", elements->semiMajorAxisKm);
    ImGui::Text("Eccentricity: %.10f", elements->eccentricity);
    ImGui::Text("Inclination: %.6f deg", elements->inclinationRad * kDegreesPerRadian);
    ImGui::Text("RAAN: %.6f deg", elements->longitudeAscendingNodeRad * kDegreesPerRadian);
    ImGui::Text("Argument of periapsis: %.6f deg", elements->argumentPeriapsisRad * kDegreesPerRadian);
    ImGui::Text("True anomaly at epoch: %.6f deg", elements->trueAnomalyRad * kDegreesPerRadian);
    optionalDistance("Periapsis", elements->periapsisRadiusKm(), spacecraft->centralBodyRadiusKm());
    optionalDistance("Apoapsis", elements->apoapsisRadiusKm(), spacecraft->centralBodyRadiusKm());
    duration("Period", elements->periodSeconds(spacecraft->orbit().muKm3PerSec2()));

    ImGui::SeparatorText("Current state");
    ImGui::Text("r: [%.6f, %.6f, %.6f] km", vector.positionKm.x, vector.positionKm.y, vector.positionKm.z);
    ImGui::Text("v: [%.9f, %.9f, %.9f] km/s", vector.velocityKmPerSec.x,
                vector.velocityKmPerSec.y, vector.velocityKmPerSec.z);
    ImGui::Text("|r|: %.6f km", glm::length(vector.positionKm));
    ImGui::Text("|v|: %.9f km/s", glm::length(vector.velocityKmPerSec));
    if (const auto energy = astrodynamics::specificOrbitalEnergy(
            vector, spacecraft->orbit().muKm3PerSec2())) {
        ImGui::Text("Specific energy: %.9f km^2/s^2", *energy);
    }
    const glm::dvec3 h = astrodynamics::specificAngularMomentumVector(vector);
    const auto e = astrodynamics::eccentricityVector(vector, spacecraft->orbit().muKm3PerSec2());
    ImGui::Text("h: [%.5f, %.5f, %.5f] km^2/s", h.x, h.y, h.z);
    if (e) ImGui::Text("e vector: [%.9f, %.9f, %.9f]", e->x, e->y, e->z);
    ImGui::TextDisabled("Elements use the central-body inertial +Z reference plane.");
    ImGui::End();
}

} // namespace solar::ui
