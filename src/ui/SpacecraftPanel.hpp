#pragma once

#include <array>
#include <memory>
#include <string>

namespace solar::app { class AppState; }
namespace solar::astronomy { class BodyDatabase; }
namespace solar::renderer { class Renderer; }
namespace solar::scene { class SolarSystemScene; }
namespace solar::simulation { class SimulationClock; class Spacecraft; }

namespace solar::ui {

class SpacecraftPanel final {
public:
    void render(app::AppState& state,
                std::unique_ptr<simulation::Spacecraft>& spacecraft,
                const astronomy::BodyDatabase& bodyDatabase,
                simulation::SimulationClock& clock,
                scene::SolarSystemScene& scene,
                renderer::Renderer& renderer);

private:
    bool m_stateVectorMode{false};
    int m_bodyIndex{3};
    std::array<char, 64> m_name{"Earth Test Vehicle"};
    double m_periapsisAltitudeKm{200.0};
    double m_apoapsisAltitudeKm{200.0};
    double m_inclinationDeg{28.5};
    double m_raanDeg{};
    double m_argumentPeriapsisDeg{};
    double m_trueAnomalyDeg{};
    std::array<double, 3> m_positionKm{6578.137, 0.0, 0.0};
    std::array<double, 3> m_velocityKmPerSec{0.0, 7.784, 0.0};
    std::string m_status;
};

} // namespace solar::ui
