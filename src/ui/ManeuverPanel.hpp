#pragma once

namespace solar::app { class AppState; }
namespace solar::simulation { class SimulationClock; class Spacecraft; }

namespace solar::ui {

class ManeuverPanel final {
public:
    void render(app::AppState& state,
                simulation::Spacecraft* spacecraft,
                simulation::SimulationClock& clock);

private:
    double m_tangentialMps{100.0};
    double m_normalMps{};
    double m_radialMps{};
    double m_targetOppositeApsisAltitudeKm{1000.0};
    double m_hohmannTargetAltitudeKm{1000.0};
    double m_targetInclinationDeg{30.0};
};

} // namespace solar::ui
