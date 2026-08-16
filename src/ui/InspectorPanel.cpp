#include "ui/InspectorPanel.hpp"

#include "app/AppState.hpp"
#include "renderer/Camera.hpp"
#include "renderer/Renderer.hpp"

#include <imgui.h>

namespace solar::ui {

void InspectorPanel::render(app::AppState& state, renderer::Renderer& renderer) const
{
    if (!state.panels.inspector) {
        return;
    }
    if (!ImGui::Begin("Inspector", &state.panels.inspector)) {
        ImGui::End();
        return;
    }

    renderer::Camera& camera = renderer.camera();
    const auto& position = camera.position();
    const auto& target = camera.target();
    ImGui::TextUnformatted("Camera");
    ImGui::Separator();
    ImGui::Text("Position  %.3f  %.3f  %.3f", position.x, position.y, position.z);
    ImGui::Text("Target    %.3f  %.3f  %.3f", target.x, target.y, target.z);
    ImGui::Text("Yaw       %.2f deg", camera.yaw());
    ImGui::Text("Pitch     %.2f deg", camera.pitch());
    ImGui::Text("Distance  %.3f", camera.distance());
    const auto forward = camera.forward();
    ImGui::Text("Forward   %.3f  %.3f  %.3f", forward.x, forward.y, forward.z);

    float fieldOfView = camera.fieldOfView();
    if (ImGui::SliderFloat("FOV", &fieldOfView, 20.0F, 120.0F, "%.1f deg")) {
        camera.setFieldOfView(fieldOfView);
    }
    ImGui::Text("Clip      %.3f / %.0f", camera.nearPlane(), camera.farPlane());
    ImGui::Spacing();
    ImGui::TextUnformatted("Viewport");
    ImGui::Separator();
    ImGui::Text("Size      %d x %d", state.viewport.width, state.viewport.height);
    ImGui::Text("FPS       %.1f", state.framesPerSecond);
    ImGui::Text("Frame     %.3f ms", state.frameTimeMilliseconds);
    ImGui::End();
}

} // namespace solar::ui
