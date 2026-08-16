#pragma once

namespace solar::app { class AppState; }
namespace solar::simulation { class Spacecraft; }

namespace solar::ui {

class OrbitPanel final {
public:
    void render(app::AppState& state, const simulation::Spacecraft* spacecraft) const;
};

} // namespace solar::ui
