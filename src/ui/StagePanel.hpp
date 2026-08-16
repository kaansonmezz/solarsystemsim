#pragma once

#include "rocket/Engine.hpp"

#include <cstddef>

namespace solar::app { class AppState; }
namespace solar::rocket { struct Vehicle; }

namespace solar::ui {

class StagePanel final {
public:
    void render(app::AppState& state,
                rocket::Vehicle& vehicle,
                std::size_t selectedStage,
                rocket::PerformanceEnvironment environment,
                double referenceGravityMetersPerSec2);
};

} // namespace solar::ui
