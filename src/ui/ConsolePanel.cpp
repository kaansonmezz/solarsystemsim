#include "ui/ConsolePanel.hpp"

#include "app/AppState.hpp"
#include "core/Logger.hpp"

#include <imgui.h>

namespace solar::ui {

void ConsolePanel::render(app::AppState& state) const
{
    if (!state.panels.console) {
        return;
    }
    if (!ImGui::Begin("Console", &state.panels.console)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear")) {
        core::Logger::clearConsole();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Application and OpenGL messages");
    ImGui::Separator();

    ImGui::BeginChild("ConsoleMessages", {0.0F, 0.0F}, false, ImGuiWindowFlags_HorizontalScrollbar);
    const auto messages = core::Logger::consoleMessages();
    for (const auto& message : messages) {
        ImVec4 color{0.80F, 0.83F, 0.88F, 1.0F};
        if (message.level == core::LogLevel::warning) {
            color = {1.0F, 0.75F, 0.22F, 1.0F};
        } else if (message.level == core::LogLevel::error) {
            color = {1.0F, 0.30F, 0.28F, 1.0F};
        }
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(message.text.c_str());
        ImGui::PopStyleColor();
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 2.0F) {
        ImGui::SetScrollHereY(1.0F);
    }
    ImGui::EndChild();
    ImGui::End();
}

} // namespace solar::ui
