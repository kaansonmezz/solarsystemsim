#pragma once

#include "rocket/Engine.hpp"

#include <cstddef>

namespace solar::app { class AppState; }
namespace solar::rocket { class EngineDatabase; struct Vehicle; }

namespace solar::ui {

class EnginePanel final {
public:
    void render(app::AppState& state,
                rocket::Vehicle& vehicle,
                std::size_t selectedStage,
                const rocket::EngineDatabase& engineDatabase,
                rocket::PerformanceEnvironment environment);
};

} // namespace solar::ui
