#include "renderer/CameraController.hpp"

#include "renderer/Camera.hpp"

#include <cmath>

namespace solar::renderer {

CameraController::CameraController(Camera& camera) noexcept
    : m_camera(camera)
{
}

void CameraController::update(const CameraInputState& input)
{
    float deltaX = 0.0F;
    float deltaY = 0.0F;
    if (m_hasPreviousCursor) {
        deltaX = static_cast<float>(input.cursorX - m_previousCursorX);
        deltaY = static_cast<float>(input.cursorY - m_previousCursorY);
    }
    m_previousCursorX = input.cursorX;
    m_previousCursorY = input.cursorY;
    m_hasPreviousCursor = true;

    const bool middlePressedNow = input.middleMousePressed && !m_middleMouseWasPressed;
    const bool rightPressedNow = input.rightMousePressed && !m_rightMouseWasPressed;

    if (middlePressedNow) {
        m_orbitDragActive = input.viewportInteractionAllowed;
    }
    if (rightPressedNow) {
        m_panDragActive = input.viewportInteractionAllowed && !input.middleMousePressed;
    }

    if (input.viewportInteractionAllowed) {
        if (input.middleMousePressed && m_orbitDragActive && !middlePressedNow) {
            m_camera.orbit(deltaX, deltaY);
        } else if (input.rightMousePressed && m_panDragActive && !rightPressedNow) {
            m_camera.pan(deltaX, deltaY);
        }

        // Some mouse/driver combinations can emit a scroll event while the wheel
        // button is held. Orbit has precedence so an MMB drag cannot change distance.
        if (!input.middleMousePressed && std::abs(input.scrollYOffset) > 0.001F) {
            m_camera.zoom(input.scrollYOffset);
        }
    }

    if (!input.middleMousePressed) m_orbitDragActive = false;
    if (!input.rightMousePressed) m_panDragActive = false;
    m_middleMouseWasPressed = input.middleMousePressed;
    m_rightMouseWasPressed = input.rightMousePressed;
}

} // namespace solar::renderer
