#pragma once

namespace solar::app { class AppState; }
namespace solar::astronomy { class SpiceContext; }
namespace solar::scene { class SolarSystemScene; }

namespace solar::ui {

class AstronomyPanel final {
public:
    void render(app::AppState& state,
                scene::SolarSystemScene& scene,
                const astronomy::SpiceContext& spiceContext) const;
};

} // namespace solar::ui

