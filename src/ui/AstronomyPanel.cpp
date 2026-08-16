#include "ui/AstronomyPanel.hpp"

#include "app/AppState.hpp"
#include "astronomy/ReferenceFrame.hpp"
#include "astronomy/SpiceContext.hpp"
#include "core/Time.hpp"
#include "scene/SolarSystemScene.hpp"

#include <imgui.h>

namespace solar::ui {

void AstronomyPanel::render(
    app::AppState& state,
    scene::SolarSystemScene& scene,
    const astronomy::SpiceContext& spiceContext) const
{
    if (!state.panels.astronomy) return;
    if (!ImGui::Begin("Astronomy", &state.panels.astronomy)) {
        ImGui::End();
        return;
    }
    const bool ready = scene.astronomyReady();
    ImGui::TextColored(ready ? ImVec4{0.3F, 0.9F, 0.4F, 1.0F} : ImVec4{1.0F, 0.35F, 0.25F, 1.0F},
                       "SPICE: %s", ready ? "READY" : "ERROR");
    ImGui::Text("Toolkit: %s", spiceContext.toolkitVersion().c_str());
    ImGui::Text("UTC: %s", core::formatUtcIso8601(scene.currentUtc()).c_str());
    ImGui::Text("ET: %.6f TDB seconds past J2000", scene.currentEphemerisTime());
    ImGui::TextWrapped("Status: %s", scene.statusMessage().c_str());

    ImGui::SeparatorText("Reference");
    int origin = static_cast<int>(scene.referenceFrame().origin);
    constexpr const char* origins[]{"Solar System Barycenter", "Sun", "Earth", "Selected Body"};
    if (ImGui::Combo("Origin", &origin, origins, 4)) {
        scene.setReferenceOrigin(static_cast<astronomy::OriginMode>(origin));
    }
    int axes = static_cast<int>(scene.referenceFrame().axes);
    constexpr const char* frames[]{"J2000", "ECLIPJ2000"};
    if (ImGui::Combo("Axes", &axes, frames, 2)) {
        scene.setAxisFrame(static_cast<astronomy::AxisFrame>(axes));
    }
    ImGui::TextUnformatted("Grid plane: selected inertial frame X-Y plane");
    if (scene.referenceFrame().axes == astronomy::AxisFrame::eclipticJ2000) {
        ImGui::TextWrapped(
            "ECLIPJ2000 uses the J2000 mean ecliptic and equinox; inclination is "
            "measured from its X-Y plane.");
    } else {
        ImGui::TextWrapped(
            "J2000 is Earth mean equator and equinox at J2000; inclination is "
            "measured from its X-Y plane.");
    }
    ImGui::TextUnformatted("Position: km   Velocity: km/s   Angles: radians");

    ImGui::SeparatorText("Loaded kernels");
    for (const auto& kernel : spiceContext.loadedKernels()) {
        ImGui::BulletText("%s", kernel.filename().string().c_str());
    }
    ImGui::TextDisabled("DE442s coverage: 1849-12-26 to 2150-01-22");
    ImGui::End();
}

} // namespace solar::ui
