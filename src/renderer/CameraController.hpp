#pragma once

namespace solar::renderer {

class Camera;

struct CameraInputState final {
    double cursorX{0.0};
    double cursorY{0.0};
    float scrollYOffset{0.0F};
    bool viewportInteractionAllowed{false};
    bool middleMousePressed{false};
    bool rightMousePressed{false};
};

class CameraController final {
public:
    explicit CameraController(Camera& camera) noexcept;

    void update(const CameraInputState& input);

private:
    Camera& m_camera;
    double m_previousCursorX{0.0};
    double m_previousCursorY{0.0};
    bool m_hasPreviousCursor{false};
    bool m_middleMouseWasPressed{false};
    bool m_rightMouseWasPressed{false};
    bool m_orbitDragActive{false};
    bool m_panDragActive{false};
};

} // namespace solar::renderer
