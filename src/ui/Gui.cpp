#include "ui/Gui.hpp"

#include "app/AppState.hpp"
#include "renderer/Renderer.hpp"
#include "simulation/SimulationClock.hpp"
#include "astronomy/SpiceContext.hpp"
#include "astronomy/BodyDatabase.hpp"
#include "scene/SolarSystemScene.hpp"
#include "rocket/EngineDatabase.hpp"
#include "rocket/Vehicle.hpp"

#include <imgui.h>
#include <implot.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <stdexcept>

namespace solar::ui {

Gui::~Gui() { shutdown(); }

void Gui::initialize(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = "solar_imgui.ini";

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0F;
    style.FrameRounding = 3.0F;
    style.TabRounding = 3.0F;

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        throw std::runtime_error("Dear ImGui GLFW backend initialization failed");
    }
    if (!ImGui_ImplOpenGL3_Init("#version 460 core")) {
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        throw std::runtime_error("Dear ImGui OpenGL backend initialization failed");
    }
    m_initialized = true;
}

void Gui::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Gui::renderPanels(
    app::AppState& state,
    renderer::Renderer& renderer,
    simulation::SimulationClock& clock,
    scene::SolarSystemScene& scene,
    const astronomy::SpiceContext& spiceContext,
    const astronomy::BodyDatabase& bodyDatabase,
    std::unique_ptr<simulation::Spacecraft>& spacecraft,
    rocket::Vehicle& rocketVehicle,
    const rocket::EngineDatabase& engineDatabase)
{
    m_dockspace.render(state, clock);
    m_viewportPanel.render(state, renderer);
    m_solarSystemPanel.render(state, scene, renderer);
    m_spacecraftPanel.render(state, spacecraft, bodyDatabase, clock, scene, renderer);
    m_orbitPanel.render(state, spacecraft.get());
    m_maneuverPanel.render(state, spacecraft.get(), clock);
    m_rocketPanel.render(state, rocketVehicle, engineDatabase, bodyDatabase);
    m_bodyInspectorPanel.render(state, scene);
    m_astronomyPanel.render(state, scene, spiceContext);
    m_inspectorPanel.render(state, renderer);
    m_timePanel.render(state, clock);
    m_consolePanel.render(state);
}

void Gui::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::shutdown()
{
    if (!m_initialized) {
        return;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    m_initialized = false;
}

} // namespace solar::ui
