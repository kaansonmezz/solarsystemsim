#pragma once

namespace solar::app { class AppState; }
namespace solar::renderer { class Renderer; }

namespace solar::ui {

class ViewportPanel final {
public:
    void render(app::AppState& state, const renderer::Renderer& renderer) const;
};

} // namespace solar::ui

