#include "ui/Dockspace.hpp"

#include "app/AppState.hpp"
#include "simulation/SimulationClock.hpp"

#include <imgui.h>
#include <imgui_internal.h>

namespace solar::ui {

void Dockspace::render(app::AppState& state, simulation::SimulationClock& clock)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0F, 0.0F});
    ImGui::Begin("SOLAR Dockspace", nullptr, flags);
    ImGui::PopStyleVar(3);

    renderMenuBar(state, clock);

    const ImGuiID dockspaceId = ImGui::GetID("SOLAR Main Dockspace");
    if (ImGui::DockBuilderGetNode(dockspaceId) == nullptr) {
        buildDefaultLayout(dockspaceId);
    }
    ImGui::DockSpace(dockspaceId, {0.0F, 0.0F}, ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    if (state.showAbout) {
        ImGui::OpenPopup("About SOLAR");
        state.showAbout = false;
    }
    if (ImGui::BeginPopupModal("About SOLAR", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("SOLAR 0.4.0");
        ImGui::Separator();
        ImGui::TextUnformatted("A scientific visualization and mission-planning foundation.");
        ImGui::Spacing();
        if (ImGui::Button("Close", {120.0F, 0.0F})) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Dockspace::renderMenuBar(app::AppState& state, simulation::SimulationClock& clock)
{
    if (!ImGui::BeginMenuBar()) {
        return;
    }
    ImGui::TextUnformatted("SOLAR");
    ImGui::Separator();

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Exit", "Esc")) state.requestExit = true;
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Viewport", nullptr, &state.panels.viewport);
        ImGui::MenuItem("Inspector", nullptr, &state.panels.inspector);
        ImGui::MenuItem("Simulation Time", nullptr, &state.panels.time);
        ImGui::MenuItem("Console", nullptr, &state.panels.console);
        ImGui::MenuItem("Solar System", nullptr, &state.panels.solarSystem);
        ImGui::MenuItem("Body Inspector", nullptr, &state.panels.bodyInspector);
        ImGui::MenuItem("Astronomy", nullptr, &state.panels.astronomy);
        ImGui::MenuItem("Spacecraft", nullptr, &state.panels.spacecraft);
        ImGui::MenuItem("Orbit", nullptr, &state.panels.orbit);
        ImGui::MenuItem("Maneuver", nullptr, &state.panels.maneuver);
        ImGui::MenuItem("ROCKET / VEHICLE", nullptr, &state.panels.rocket);
        ImGui::MenuItem("Rocket Stage", nullptr, &state.panels.stage);
        ImGui::MenuItem("Rocket Engine", nullptr, &state.panels.engine);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Simulation")) {
        if (ImGui::MenuItem("Pause", "Space", false, !clock.isPaused())) clock.pause();
        if (ImGui::MenuItem("Resume", "Space", false, clock.isPaused())) clock.resume();
        if (ImGui::MenuItem("Real Time", "R")) clock.returnToRealTime();
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Tools")) {
        ImGui::MenuItem("OpenGL notifications", nullptr, &state.showOpenGlNotifications);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("About")) state.showAbout = true;
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

void Dockspace::buildDefaultLayout(const unsigned int dockspaceId)
{
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->WorkSize);

    ImGuiID mainId = dockspaceId;
    const ImGuiID bottomId = ImGui::DockBuilderSplitNode(
        mainId, ImGuiDir_Down, 0.24F, nullptr, &mainId);
    const ImGuiID leftId = ImGui::DockBuilderSplitNode(
        mainId, ImGuiDir_Left, 0.23F, nullptr, &mainId);
    ImGuiID leftTopId = leftId;
    const ImGuiID leftBottomId = ImGui::DockBuilderSplitNode(
        leftTopId, ImGuiDir_Down, 0.46F, nullptr, &leftTopId);

    ImGui::DockBuilderDockWindow("Viewport", mainId);
    ImGui::DockBuilderDockWindow("Inspector", leftTopId);
    ImGui::DockBuilderDockWindow("Solar System", leftTopId);
    ImGui::DockBuilderDockWindow("Body Inspector", leftTopId);
    ImGui::DockBuilderDockWindow("Simulation Time", leftBottomId);
    ImGui::DockBuilderDockWindow("Astronomy", leftBottomId);
    ImGui::DockBuilderDockWindow("Spacecraft", leftTopId);
    ImGui::DockBuilderDockWindow("Orbit", leftTopId);
    ImGui::DockBuilderDockWindow("Maneuver", leftBottomId);
    ImGui::DockBuilderDockWindow("ROCKET / VEHICLE", leftTopId);
    ImGui::DockBuilderDockWindow("Rocket Stage", leftBottomId);
    ImGui::DockBuilderDockWindow("Rocket Engine", leftBottomId);
    ImGui::DockBuilderDockWindow("Console", bottomId);
    ImGui::DockBuilderFinish(dockspaceId);
}

} // namespace solar::ui
