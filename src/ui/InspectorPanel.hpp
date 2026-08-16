#pragma once

namespace solar::app { class AppState; }
namespace solar::renderer { class Renderer; }

namespace solar::ui {

class InspectorPanel final {
public:
    void render(app::AppState& state, renderer::Renderer& renderer) const;
};

} // namespace solar::ui

