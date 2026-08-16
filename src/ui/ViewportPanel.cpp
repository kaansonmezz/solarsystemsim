#include "ui/ViewportPanel.hpp"

#include "app/AppState.hpp"
#include "renderer/Renderer.hpp"

#include <imgui.h>

#include <algorithm>
#include <cstdint>

namespace solar::ui {

void ViewportPanel::render(app::AppState& state, const renderer::Renderer& renderer) const
{
    if (!state.panels.viewport) {
        state.viewport.hovered = false;
        state.viewport.focused = false;
        return;
    }

    if (ImGui::Begin("Viewport", &state.panels.viewport, ImGuiWindowFlags_NoScrollbar |
                                                         ImGuiWindowFlags_NoScrollWithMouse)) {
        const ImVec2 available = ImGui::GetContentRegionAvail();
        state.viewport.width = std::max(static_cast<int>(available.x), 1);
        state.viewport.height = std::max(static_cast<int>(available.y), 1);

        const auto textureId = static_cast<ImTextureID>(
            static_cast<std::uintptr_t>(renderer.viewportTexture()));
        ImGui::Image(textureId, available, {0.0F, 1.0F}, {1.0F, 0.0F});
        const ImVec2 imageMinimum = ImGui::GetItemRectMin();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        for (const auto& label : renderer.screenLabels()) {
            const ImVec2 position{
                imageMinimum.x + label.normalizedPosition.x * available.x + 6.0F,
                imageMinimum.y + label.normalizedPosition.y * available.y - 8.0F,
            };
            const ImU32 color = ImGui::ColorConvertFloat4ToU32(
                {label.color.r, label.color.g, label.color.b, 1.0F});
            drawList->AddText(position, color, label.text.c_str());
        }
        state.viewport.hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            const ImVec2 mousePosition = ImGui::GetIO().MousePos;
            state.viewport.clickNormalizedX = std::clamp(
                (mousePosition.x - imageMinimum.x) / std::max(available.x, 1.0F), 0.0F, 1.0F);
            state.viewport.clickNormalizedY = std::clamp(
                (mousePosition.y - imageMinimum.y) / std::max(available.y, 1.0F), 0.0F, 1.0F);
            state.viewport.leftClickPending = true;
        }
        state.viewport.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    } else {
        state.viewport.hovered = false;
        state.viewport.focused = false;
    }
    ImGui::End();
}

} // namespace solar::ui
