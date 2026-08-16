#pragma once

#include "core/Time.hpp"

#include <string>

namespace solar::app { class AppState; }
namespace solar::simulation { class SimulationClock; }

namespace solar::ui {

class TimePanel final {
public:
    void render(app::AppState& state, simulation::SimulationClock& clock);

private:
    core::UtcComponents m_inputUtc;
    std::string m_dateError;
    bool m_dateInitialized{false};
};

} // namespace solar::ui
