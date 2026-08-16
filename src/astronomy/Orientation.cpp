#include "astronomy/Orientation.hpp"

#include "astronomy/CelestialBody.hpp"
#include "astronomy/SpiceContext.hpp"

namespace solar::astronomy {

Orientation::Orientation(SpiceContext& spiceContext) : m_spiceContext(spiceContext) {}

std::optional<glm::dmat3> Orientation::bodyFixedToReference(
    const CelestialBody& body,
    const AxisFrame referenceAxes,
    const double ephemerisTime) const
{
    return m_spiceContext.frameTransform(
        body.bodyFixedFrame(), name(referenceAxes), ephemerisTime);
}

} // namespace solar::astronomy
