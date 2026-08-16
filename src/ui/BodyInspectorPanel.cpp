#include "ui/BodyInspectorPanel.hpp"

#include "app/AppState.hpp"
#include "astronomy/CelestialBody.hpp"
#include "astronomy/ReferenceFrame.hpp"
#include "astrodynamics/OrbitalElements.hpp"
#include "scene/RenderTransform.hpp"
#include "scene/SolarSystemScene.hpp"

#include <imgui.h>
#include <glm/geometric.hpp>

#include <cmath>
#include <numbers>
#include <string>

namespace solar::ui {
namespace {

[[nodiscard]] std::string groupedKilometres(const double kilometres)
{
    std::string result = std::to_string(static_cast<long long>(std::llround(kilometres)));
    const std::size_t firstDigit = !result.empty() && result.front() == '-' ? 1U : 0U;
    for (std::size_t position = result.size(); position > firstDigit + 3U; position -= 3U) {
        result.insert(position - 3U, 1, ',');
    }
    return result + " km";
}

void distanceText(const char* label, const double kilometres)
{
    if (!std::isfinite(kilometres)) {
        ImGui::Text("%-25s %s", label, "Not defined");
        return;
    }
    const std::string km = groupedKilometres(kilometres);
    if (std::abs(kilometres) >= scene::kAstronomicalUnitKm * 0.1) {
        ImGui::Text("%-25s %s  (%.6f AU)", label, km.c_str(),
                    kilometres / scene::kAstronomicalUnitKm);
        return;
    }
    ImGui::Text("%-25s %s", label, km.c_str());
}

void angleText(const char* label, const double radians)
{
    ImGui::Text("%-25s %.6f deg", label, radians * 180.0 / std::numbers::pi);
}

} // namespace

void BodyInspectorPanel::render(
    app::AppState& state, scene::SolarSystemScene& scene) const
{
    if (!state.panels.bodyInspector) return;
    if (!ImGui::Begin("Body Inspector", &state.panels.bodyInspector)) {
        ImGui::End();
        return;
    }
    const auto* object = scene.selectedObject();
    if (object == nullptr) {
        ImGui::TextUnformatted("No body selected");
        ImGui::End();
        return;
    }
    const auto& body = *object->body;
    ImGui::Text("%s", body.name().c_str());
    ImGui::Text("NAIF ID: %d  Ephemeris center: %d", body.naifId(), body.ephemerisId());

    ImGui::SeparatorText("Physical");
    distanceText("Mean radius", body.meanRadiusKm());
    ImGui::Text("%-20s %.6e kg", "Mass", body.massKg());
    ImGui::Text("%-20s %.6f km^3/s^2", "mu", body.gravitationalParameterKm3PerSec2());
    ImGui::Text("%-20s %.4f m/s^2", "Surface gravity", body.surfaceGravityMetersPerSec2());
    ImGui::Text("%-20s %.4f km/s", "Escape velocity", body.escapeVelocityKmPerSec());

    ImGui::SeparatorText("Rotation / Orientation");
    ImGui::Text("Period: %.6f hours", body.rotationPeriodSeconds() / 3'600.0);
    ImGui::Text("Body-fixed frame: %s", body.bodyFixedFrame().c_str());
    ImGui::TextUnformatted("Orientation evaluated from SPICE at current ET.");
    ImGui::TextUnformatted("Rotation axis is independent of the orbital normal.");

    ImGui::SeparatorText("Current Osculating Orbit");
    if (object->osculatingOrbit && object->orbitCentralBody != nullptr) {
        const auto& orbit = *object->osculatingOrbit;
        const auto& elements = orbit.elements;
        ImGui::Text("%-25s %s", "Central Body", object->orbitCentralBody->name().c_str());
        ImGui::Text("%-25s %s X-Y plane", "Reference Plane",
                    astronomy::name(scene.referenceFrame().axes).data());
        ImGui::Text("%-25s %s", "Orbit Type", astrodynamics::name(elements.type()).data());
        distanceText("Semi-major Axis", elements.semiMajorAxisKm);
        ImGui::Text("%-25s %.9f", "Eccentricity", elements.eccentricity);
        angleText("Inclination", elements.inclinationRad);
        angleText("Longitude Ascending Node", elements.longitudeAscendingNodeRad);
        angleText("Argument of Periapsis", elements.argumentPeriapsisRad);
        angleText("True Anomaly", elements.trueAnomalyRad);
        distanceText("Periapsis", elements.periapsisRadiusKm());
        if (const auto apoapsis = elements.apoapsisRadiusKm()) {
            distanceText("Apoapsis", *apoapsis);
        } else {
            ImGui::Text("%-25s %s", "Apoapsis", "Not defined");
        }
        if (const auto period = elements.periodSeconds(
                object->orbitCentralBody->gravitationalParameterKm3PerSec2())) {
            const double days = *period / 86'400.0;
            if (days >= 730.0) {
                ImGui::Text("%-25s %.6f days  (%.6f years)", "Orbital Period", days,
                            days / 365.25);
            } else {
                ImGui::Text("%-25s %.6f days", "Orbital Period", days);
            }
        } else {
            ImGui::Text("%-25s %s", "Orbital Period", "Not defined");
        }
        ImGui::TextWrapped(
            "Derived from the current CSPICE r(t), v(t). These osculating elements "
            "vary with epoch and are not permanent body parameters.");

        ImGui::SeparatorText("Orbital Geometry Controls");
        ImGui::Checkbox("Show Orbit", &scene.showOrbits);
        ImGui::Checkbox("Show Orbital Plane", &scene.showOrbitalPlane);
        ImGui::Checkbox("Show Reference Plane", &scene.showReferencePlane);
        ImGui::Checkbox("Show Orbital Normal", &scene.showOrbitalNormal);
        ImGui::Checkbox("Show Ascending / Descending Nodes", &scene.showOrbitNodes);
        ImGui::Checkbox("Show Rotation Axis", &scene.showRotationAxis);
        ImGui::Checkbox("Show Equatorial Plane", &scene.showEquatorialPlane);

        if (ImGui::TreeNode("Scientific orbit vectors")) {
            const auto& position = orbit.relativeState.positionKm;
            const auto& velocity = orbit.relativeState.velocityKmPerSec;
            const auto& angularMomentum = orbit.angularMomentumKm2PerSec;
            ImGui::Text("Position relative to %s (km)", object->orbitCentralBody->name().c_str());
            ImGui::Text("X  % .9e", position.x);
            ImGui::Text("Y  % .9e", position.y);
            ImGui::Text("Z  % .9e", position.z);
            ImGui::Text("Velocity relative to %s (km/s)", object->orbitCentralBody->name().c_str());
            ImGui::Text("VX % .12e", velocity.x);
            ImGui::Text("VY % .12e", velocity.y);
            ImGui::Text("VZ % .12e", velocity.z);
            ImGui::TextUnformatted("Angular momentum h = r x v (km^2/s)");
            ImGui::Text("HX % .12e", angularMomentum.x);
            ImGui::Text("HY % .12e", angularMomentum.y);
            ImGui::Text("HZ % .12e", angularMomentum.z);
            ImGui::TreePop();
        }
    } else {
        ImGui::TextUnformatted("Osculating orbit unavailable");
    }

    ImGui::SeparatorText("Current Scene State");
    const auto& position = object->referenceState.positionKm;
    const auto& velocity = object->referenceState.velocityKmPerSec;
    ImGui::Text("Position X  % .6e km", position.x);
    ImGui::Text("Position Y  % .6e km", position.y);
    ImGui::Text("Position Z  % .6e km", position.z);
    ImGui::Text("Velocity X  % .9f km/s", velocity.x);
    ImGui::Text("Velocity Y  % .9f km/s", velocity.y);
    ImGui::Text("Velocity Z  % .9f km/s", velocity.z);
    ImGui::Text("Speed       %.6f km/s", glm::length(velocity));
    distanceText("Distance from Sun", object->distanceFromSunKm);
    distanceText("Distance from Earth", object->distanceFromEarthKm);

    ImGui::SeparatorText("Scene Reference");
    if (scene.referenceFrame().origin == astronomy::OriginMode::selectedBody) {
        ImGui::Text("Reference Origin: Selected Body (%s)", body.name().c_str());
    } else {
        ImGui::Text("Reference Origin: %s", astronomy::name(scene.referenceFrame().origin).data());
    }
    ImGui::Text("Axes: %s", astronomy::name(scene.referenceFrame().axes).data());
    ImGui::Text("Reference Plane: %s X-Y plane",
                astronomy::name(scene.referenceFrame().axes).data());
    ImGui::TextUnformatted("State: geometric (aberration correction NONE)");

    ImGui::SeparatorText("Render");
    ImGui::Text("Position: %.3f, %.3f, %.3f", object->renderPosition.x,
                object->renderPosition.y, object->renderPosition.z);
    ImGui::Text("Display radius: %.4f units", object->renderRadius);
    ImGui::Text("Radius scale: %.0fx (visual only)", scene.renderTransform().bodyRadiusScale());
    ImGui::End();
}

} // namespace solar::ui
