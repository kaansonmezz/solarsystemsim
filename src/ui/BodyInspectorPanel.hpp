#pragma once

namespace solar::app { class AppState; }
namespace solar::scene { class SolarSystemScene; }

namespace solar::ui {

class BodyInspectorPanel final {
public:
    void render(app::AppState& state, scene::SolarSystemScene& scene) const;
};

} // namespace solar::ui
