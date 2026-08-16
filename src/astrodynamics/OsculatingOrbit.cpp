#include "astrodynamics/OsculatingOrbit.hpp"

#include "astrodynamics/Angles.hpp"
#include "astrodynamics/OrbitConversions.hpp"
#include "astrodynamics/Tolerances.hpp"

#include <glm/geometric.hpp>

#include <cmath>
#include <numbers>

namespace solar::astrodynamics {

std::optional<OsculatingOrbitData> deriveOsculatingOrbit(
    const astronomy::StateVector& relativeState,
    const double centralBodyMuKm3PerSec2) noexcept
{
    const auto elements = stateToOrbitalElements(relativeState, centralBodyMuKm3PerSec2);
    if (!elements) return std::nullopt;

    const glm::dvec3 angularMomentum = glm::cross(
        relativeState.positionKm, relativeState.velocityKmPerSec);
    const double angularMomentumMagnitude = glm::length(angularMomentum);
    const double radius = glm::length(relativeState.positionKm);
    if (!(angularMomentumMagnitude > kVectorTolerance) || !(radius > kVectorTolerance)) {
        return std::nullopt;
    }

    const glm::dvec3 normal = angularMomentum / angularMomentumMagnitude;
    const glm::dvec3 node = glm::cross(glm::dvec3{0.0, 0.0, 1.0}, angularMomentum);
    const double nodeMagnitude = glm::length(node);
    const bool hasDefinedNodes = nodeMagnitude > kVectorTolerance &&
        elements->inclinationRad > kEquatorialToleranceRad &&
        std::abs(elements->inclinationRad - std::numbers::pi) > kEquatorialToleranceRad;

    const glm::dvec3 basisU = hasDefinedNodes
        ? node / nodeMagnitude
        : relativeState.positionKm / radius;
    const glm::dvec3 basisV = glm::normalize(glm::cross(normal, basisU));

    std::optional<glm::dvec3> ascendingNode;
    std::optional<glm::dvec3> descendingNode;
    if (hasDefinedNodes) {
        OrbitalElements nodeElements = *elements;
        nodeElements.trueAnomalyRad = normalizeAnglePositive(
            -nodeElements.argumentPeriapsisRad);
        if (const auto state = orbitalElementsToState(nodeElements, centralBodyMuKm3PerSec2)) {
            ascendingNode = state->positionKm;
        }
        nodeElements.trueAnomalyRad = normalizeAnglePositive(
            std::numbers::pi - nodeElements.argumentPeriapsisRad);
        if (const auto state = orbitalElementsToState(nodeElements, centralBodyMuKm3PerSec2)) {
            descendingNode = state->positionKm;
        }
    }

    return OsculatingOrbitData{
        relativeState,
        *elements,
        angularMomentum,
        normal,
        basisU,
        basisV,
        ascendingNode,
        descendingNode,
    };
}

} // namespace solar::astrodynamics
