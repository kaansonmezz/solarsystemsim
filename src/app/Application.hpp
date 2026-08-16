#pragma once

#include "app/AppState.hpp"
#include "app/AppConfig.hpp"
#include "simulation/SimulationClock.hpp"

#include <chrono>
#include <memory>
#include <string>

struct GLFWwindow;

namespace solar::renderer {
class CameraController;
class Renderer;
}
namespace solar::ui { class Gui; }
namespace solar::astronomy {
class BodyDatabase;
class Ephemeris;
class Orientation;
class SpiceContext;
}
namespace solar::scene { class SolarSystemScene; }
namespace solar::simulation { class Spacecraft; }
namespace solar::rocket { class EngineDatabase; struct Vehicle; }

namespace solar::app {

class Application final {
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run(int argc, char** argv);

private:
    void initialize(int argc, char** argv);
    void shutdown() noexcept;
    void loadConfig();
    void processInput(float realDeltaSeconds);
    void focusClickedBody();
    void updateFrameStatistics(double realDeltaSeconds);

    static void glfwErrorCallback(int error, const char* description);
    static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);

    GLFWwindow* m_window{nullptr};
    std::unique_ptr<renderer::Renderer> m_renderer;
    std::unique_ptr<renderer::CameraController> m_cameraController;
    std::unique_ptr<ui::Gui> m_gui;
    std::unique_ptr<astronomy::SpiceContext> m_spiceContext;
    std::unique_ptr<astronomy::BodyDatabase> m_bodyDatabase;
    std::unique_ptr<astronomy::Ephemeris> m_ephemeris;
    std::unique_ptr<astronomy::Orientation> m_orientation;
    std::unique_ptr<scene::SolarSystemScene> m_solarSystemScene;
    std::unique_ptr<simulation::Spacecraft> m_spacecraft;
    std::unique_ptr<rocket::Vehicle> m_rocketVehicle;
    std::unique_ptr<rocket::EngineDatabase> m_engineDatabase;
    simulation::SimulationClock m_simulationClock;
    AppState m_state;
    AppConfig m_config;
    std::chrono::steady_clock::time_point m_previousFrameTime;
    double m_scrollDelta{0.0};
    bool m_glfwInitialized{false};
    bool m_initialized{false};
};

} // namespace solar::app
