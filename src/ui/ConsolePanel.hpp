#pragma once

namespace solar::app { class AppState; }

namespace solar::ui {

class ConsolePanel final {
public:
    void render(app::AppState& state) const;
};

} // namespace solar::ui

