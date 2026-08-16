#pragma once

namespace solar::app {

struct PanelVisibility final {
    bool viewport{true};
    bool inspector{true};
    bool time{true};
    bool console{true};
    bool solarSystem{true};
    bool astronomy{true};
    bool bodyInspector{true};
    bool spacecraft{true};
    bool orbit{true};
    bool maneuver{true};
    bool rocket{true};
    bool stage{true};
    bool engine{true};
};

struct ViewportState final {
    int width{1};
    int height{1};
    bool hovered{false};
    bool focused{false};
    bool leftClickPending{false};
    float clickNormalizedX{0.0F};
    float clickNormalizedY{0.0F};
};

class AppState final {
public:
    PanelVisibility panels;
    ViewportState viewport;
    bool showAbout{false};
    bool requestExit{false};
    bool showOpenGlNotifications{false};
    bool followSpacecraft{false};
    double framesPerSecond{0.0};
    double frameTimeMilliseconds{0.0};
};

} // namespace solar::app
