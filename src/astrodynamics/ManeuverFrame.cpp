#include "astrodynamics/ManeuverFrame.hpp"

#include "astrodynamics/Tolerances.hpp"

#include <glm/geometric.hpp>

namespace solar::astrodynamics {

std::optional<ManeuverFrame> makeManeuverFrame(const astronomy::StateVector& state) noexcept
{
    const double positionLength = glm::length(state.positionKm);
    const glm::dvec3 angularMomentum = glm::cross(state.positionKm, state.velocityKmPerSec);
    const double momentumLength = glm::length(angularMomentum);
    if (positionLength <= kVectorTolerance || momentumLength <= kVectorTolerance) {
        return std::nullopt;
    }
    const glm::dvec3 radialOut = state.positionKm / positionLength;
    const glm::dvec3 normal = angularMomentum / momentumLength;
    const glm::dvec3 tangential = glm::normalize(glm::cross(normal, radialOut));
    return ManeuverFrame{
        tangential,
        normal,
        radialOut,
    };
}

} // namespace solar::astrodynamics
