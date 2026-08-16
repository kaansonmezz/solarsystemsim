#pragma once

namespace solar::app { class AppState; }
namespace solar::renderer { class Renderer; }
namespace solar::scene { class SolarSystemScene; }

namespace solar::ui {

class SolarSystemPanel final {
public:
    void render(app::AppState& state,
                scene::SolarSystemScene& scene,
                renderer::Renderer& renderer) const;
};

} // namespace solar::ui

