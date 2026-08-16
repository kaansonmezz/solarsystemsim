#pragma once

#include "ui/AstronomyPanel.hpp"
#include "ui/BodyInspectorPanel.hpp"
#include "ui/ConsolePanel.hpp"
#include "ui/Dockspace.hpp"
#include "ui/InspectorPanel.hpp"
#include "ui/ManeuverPanel.hpp"
#include "ui/OrbitPanel.hpp"
#include "ui/RocketPanel.hpp"
#include "ui/SolarSystemPanel.hpp"
#include "ui/SpacecraftPanel.hpp"
#include "ui/TimePanel.hpp"
#include "ui/ViewportPanel.hpp"

#include <memory>

struct GLFWwindow;

namespace solar::app { class AppState; }
namespace solar::renderer { class Renderer; }
namespace solar::simulation { class SimulationClock; class Spacecraft; }
namespace solar::astronomy { class BodyDatabase; class SpiceContext; }
namespace solar::scene { class SolarSystemScene; }
namespace solar::rocket { class EngineDatabase; struct Vehicle; }

namespace solar::ui {

class Gui final {
public:
    Gui() = default;
    ~Gui();

    Gui(const Gui&) = delete;
    Gui& operator=(const Gui&) = delete;

    void initialize(GLFWwindow* window);
    void beginFrame();
    void renderPanels(app::AppState& state,
                      renderer::Renderer& renderer,
                      simulation::SimulationClock& clock,
                      scene::SolarSystemScene& scene,
                      const astronomy::SpiceContext& spiceContext,
                      const astronomy::BodyDatabase& bodyDatabase,
                      std::unique_ptr<simulation::Spacecraft>& spacecraft,
                      rocket::Vehicle& rocketVehicle,
                      const rocket::EngineDatabase& engineDatabase);
    void endFrame();
    void shutdown();

private:
    Dockspace m_dockspace;
    AstronomyPanel m_astronomyPanel;
    BodyInspectorPanel m_bodyInspectorPanel;
    ViewportPanel m_viewportPanel;
    InspectorPanel m_inspectorPanel;
    SolarSystemPanel m_solarSystemPanel;
    SpacecraftPanel m_spacecraftPanel;
    OrbitPanel m_orbitPanel;
    ManeuverPanel m_maneuverPanel;
    RocketPanel m_rocketPanel;
    TimePanel m_timePanel;
    ConsolePanel m_consolePanel;
    bool m_initialized{false};
};

} // namespace solar::ui
