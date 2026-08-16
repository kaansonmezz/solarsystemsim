#include "ui/SolarSystemPanel.hpp"

#include "app/AppState.hpp"
#include "astronomy/CelestialBody.hpp"
#include "renderer/Camera.hpp"
#include "renderer/Renderer.hpp"
#include "scene/SolarSystemScene.hpp"

#include <imgui.h>
#include <glm/geometric.hpp>

#include <algorithm>

namespace solar::ui {
namespace {

void focusSelected(scene::SolarSystemScene& scene, renderer::Renderer& renderer)
{
    const auto* selected = scene.selectedObject();
    if (selected == nullptr) return;
    scene.setReferenceOrigin(astronomy::OriginMode::selectedBody);
    scene.followSelected = true;
    renderer.camera().focus(selected->renderPosition, std::max(selected->renderRadius * 8.0F, 0.8F));
}

} // namespace

void SolarSystemPanel::render(
    app::AppState& state,
    scene::SolarSystemScene& scene,
    renderer::Renderer& renderer) const
{
    if (!state.panels.solarSystem) return;
    if (!ImGui::Begin("Solar System", &state.panels.solarSystem)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Frame Solar System")) {
        scene.setReferenceOrigin(astronomy::OriginMode::sun);
        scene.setDistanceScale(10.0);
        scene.followSelected = false;
        renderer.camera().focus(glm::vec3{0.0F}, 340.0F);
    }
    ImGui::SameLine();
    if (ImGui::Button("Earth-Moon")) {
        scene.selectBody(399);
        scene.setReferenceOrigin(astronomy::OriginMode::earth);
        scene.setDistanceScale(250.0);
        scene.followSelected = true;
        renderer.camera().focus(glm::vec3{0.0F}, 2.4F);
    }
    if (ImGui::Button("Focus Selected")) focusSelected(scene, renderer);
    ImGui::SameLine();
    ImGui::Checkbox("Follow", &scene.followSelected);

    ImGui::SeparatorText("Bodies");
    for (auto& object : scene.objects()) {
        ImGui::PushID(object.body->naifId());
        ImGui::Checkbox("##visible", &object.visible);
        ImGui::SameLine();
        const bool selected = scene.selectedBodyId() == object.body->naifId();
        if (ImGui::Selectable(object.body->name().c_str(), selected)) {
            scene.selectBody(object.body->naifId());
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            scene.selectBody(object.body->naifId());
            focusSelected(scene, renderer);
        }
        ImGui::PopID();
    }

    ImGui::SeparatorText("Visualization");
    ImGui::Checkbox("Labels", &scene.showLabels);
    ImGui::SameLine();
    ImGui::Checkbox("Show Orbit", &scene.showOrbits);
    ImGui::Checkbox("Reference-plane grid", &scene.showGrid);
    ImGui::SameLine();
    ImGui::Checkbox("XYZ axes", &scene.showAxes);
    ImGui::SeparatorText("Selected Body Geometry");
    const auto* selected = scene.selectedObject();
    const bool geometryAvailable = selected != nullptr && selected->orbitalRenderData.has_value();
    if (!geometryAvailable) ImGui::BeginDisabled();
    if (ImGui::Button("Inclination View")) {
        const auto& geometry = *selected->orbitalRenderData;
        scene.showOrbits = true;
        scene.showReferencePlane = true;
        scene.showOrbitalPlane = true;
        scene.followSelected = false;
        renderer.camera().focusFromDirection(
            geometry.centralBodyPosition, geometry.orbitalBasisU,
            std::max(geometry.planeRadius * 2.35F, 0.8F));
    }
    if (!geometryAvailable) ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip(
            "Looks along the real ascending-node direction so the reference and "
            "orbital planes are seen edge-on. Inclination is not exaggerated.");
    }
    ImGui::Checkbox("Show Orbital Plane", &scene.showOrbitalPlane);
    ImGui::Checkbox("Show Reference Plane", &scene.showReferencePlane);
    ImGui::Checkbox("Show Orbital Normal", &scene.showOrbitalNormal);
    ImGui::Checkbox("Show Ascending / Descending Nodes", &scene.showOrbitNodes);
    ImGui::Checkbox("Show Rotation Axis", &scene.showRotationAxis);
    ImGui::Checkbox("Show Equatorial Plane", &scene.showEquatorialPlane);

    double distanceScale = scene.renderTransform().distanceScale();
    constexpr double minimumDistanceScale = 0.1;
    constexpr double maximumDistanceScale = 100'000.0;
    if (ImGui::SliderScalar("Distance (units/AU)", ImGuiDataType_Double, &distanceScale,
                            &minimumDistanceScale, &maximumDistanceScale, "%.2f",
                            ImGuiSliderFlags_Logarithmic)) {
        scene.setDistanceScale(distanceScale);
    }
    double radiusScale = scene.renderTransform().bodyRadiusScale();
    constexpr double minimumRadiusScale = 1.0;
    constexpr double maximumRadiusScale = 10'000.0;
    if (ImGui::SliderScalar("Body radius scale", ImGuiDataType_Double, &radiusScale,
                            &minimumRadiusScale, &maximumRadiusScale, "%.0fx",
                            ImGuiSliderFlags_Logarithmic)) {
        scene.setBodyRadiusScale(radiusScale);
    }
    ImGui::TextDisabled("Radius scale/minimum visibility are render-only.");
    ImGui::End();
}

} // namespace solar::ui
