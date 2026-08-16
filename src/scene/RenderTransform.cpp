#include "scene/RenderTransform.hpp"

#include <algorithm>

namespace solar::scene {

void RenderTransform::setDistanceScale(const double renderUnitsPerAu) noexcept
{
    m_renderUnitsPerAu = std::clamp(renderUnitsPerAu, 0.01, 100'000.0);
}

void RenderTransform::setBodyRadiusScale(const double multiplier) noexcept
{
    m_bodyRadiusScale = std::clamp(multiplier, 1.0, 100'000.0);
}

glm::vec3 RenderTransform::toRenderPosition(const glm::dvec3& rebasedPositionKm) const noexcept
{
    const double scale = m_renderUnitsPerAu / kAstronomicalUnitKm;
    // J2000/ECLIPJ2000 X-Y plane maps to the renderer X-Z ground plane; +Z is render +Y.
    return {
        static_cast<float>(rebasedPositionKm.x * scale),
        static_cast<float>(rebasedPositionKm.z * scale),
        static_cast<float>(-rebasedPositionKm.y * scale),
    };
}

glm::vec3 RenderTransform::toRenderDirection(const glm::dvec3& scientificDirection) const noexcept
{
    return {
        static_cast<float>(scientificDirection.x),
        static_cast<float>(scientificDirection.z),
        static_cast<float>(-scientificDirection.y),
    };
}

float RenderTransform::toRenderRadius(
    const double physicalRadiusKm,
    const float visualWeight,
    const bool isSun) const noexcept
{
    const double physicalScaled = physicalRadiusKm * m_renderUnitsPerAu /
                                  kAstronomicalUnitKm * m_bodyRadiusScale;
    const double minimumVisible = static_cast<double>(visualWeight) * 0.055;
    const double maximumSunRadius = isSun ? 1.4 : 10.0;
    return static_cast<float>(std::clamp(std::max(physicalScaled, minimumVisible), 0.01, maximumSunRadius));
}

double RenderTransform::distanceScale() const noexcept { return m_renderUnitsPerAu; }
double RenderTransform::bodyRadiusScale() const noexcept { return m_bodyRadiusScale; }

} // namespace solar::scene
