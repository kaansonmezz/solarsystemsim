#pragma once

#include <glm/vec3.hpp>

namespace solar::scene {

inline constexpr double kAstronomicalUnitKm = 149'597'870.7;

class RenderTransform final {
public:
    void setDistanceScale(double renderUnitsPerAu) noexcept;
    void setBodyRadiusScale(double multiplier) noexcept;

    [[nodiscard]] glm::vec3 toRenderPosition(const glm::dvec3& rebasedPositionKm) const noexcept;
    [[nodiscard]] glm::vec3 toRenderDirection(const glm::dvec3& scientificDirection) const noexcept;
    [[nodiscard]] float toRenderRadius(double physicalRadiusKm, float visualWeight, bool isSun) const noexcept;
    [[nodiscard]] double distanceScale() const noexcept;
    [[nodiscard]] double bodyRadiusScale() const noexcept;

private:
    double m_renderUnitsPerAu{10.0};
    double m_bodyRadiusScale{100.0};
};

} // namespace solar::scene
