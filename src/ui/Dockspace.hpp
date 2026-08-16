#pragma once

namespace solar::app { class AppState; }
namespace solar::simulation { class SimulationClock; }

namespace solar::ui {

class Dockspace final {
public:
    void render(app::AppState& state, simulation::SimulationClock& clock);

private:
    void renderMenuBar(app::AppState& state, simulation::SimulationClock& clock);
    void buildDefaultLayout(unsigned int dockspaceId);
};

} // namespace solar::ui

