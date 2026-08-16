#include "astrodynamics/Maneuver.hpp"

#include "astrodynamics/ManeuverFrame.hpp"
#include "astrodynamics/OrbitalMechanics.hpp"
#include "astrodynamics/Tolerances.hpp"

#include <glm/geometric.hpp>

#include <cmath>

namespace solar::astrodynamics {

double ImpulsiveManeuver::magnitudeKmPerSec() const noexcept
{
    return std::sqrt(tangentialKmPerSec * tangentialKmPerSec +
                     normalKmPerSec * normalKmPerSec +
                     radialKmPerSec * radialKmPerSec);
}

std::optional<glm::dvec3> maneuverDeltaVelocity(
    const astronomy::StateVector& state, const ImpulsiveManeuver& maneuver) noexcept
{
    const auto frame = makeManeuverFrame(state);
    if (!frame || !std::isfinite(maneuver.magnitudeKmPerSec())) return std::nullopt;
    const glm::dvec3 deltaVelocity =
           frame->tangential * maneuver.tangentialKmPerSec +
           frame->normal * maneuver.normalKmPerSec +
           frame->radialOut * maneuver.radialKmPerSec;
    return astronomy::isFinite({{}, deltaVelocity})
        ? std::optional<glm::dvec3>{deltaVelocity} : std::nullopt;
}

std::optional<astronomy::StateVector> applyManeuver(
    const astronomy::StateVector& state, const ImpulsiveManeuver& maneuver) noexcept
{
    const auto deltaVelocity = maneuverDeltaVelocity(state, maneuver);
    if (!deltaVelocity) return std::nullopt;
    astronomy::StateVector result = state;
    result.velocityKmPerSec += *deltaVelocity;
    return astronomy::isFinite(result) ? std::optional<astronomy::StateVector>{result}
                                        : std::nullopt;
}

std::optional<double> circularizationDeltaVelocityKmPerSec(
    const astronomy::StateVector& state, const double mu) noexcept
{
    const double radius = glm::length(state.positionKm);
    const auto circularSpeed = circularVelocityKmPerSec(mu, radius);
    const auto frame = makeManeuverFrame(state);
    if (!circularSpeed || !frame) return std::nullopt;
    if (std::abs(glm::dot(state.velocityKmPerSec, frame->radialOut)) > 1.0e-7) {
        return std::nullopt;
    }
    return *circularSpeed - glm::dot(state.velocityKmPerSec, frame->tangential);
}

std::optional<double> apsidalTargetDeltaVelocityKmPerSec(
    const astronomy::StateVector& state,
    const double targetOppositeApsisRadiusKm,
    const double mu) noexcept
{
    const double radius = glm::length(state.positionKm);
    if (!(targetOppositeApsisRadiusKm > 0.0) || !(mu > 0.0) ||
        radius <= kVectorTolerance) return std::nullopt;
    const auto frame = makeManeuverFrame(state);
    if (!frame) return std::nullopt;
    // This helper deliberately requires an apsis: radial velocity must be negligible.
    const double radialVelocity = glm::dot(state.velocityKmPerSec, frame->radialOut);
    if (std::abs(radialVelocity) > 1.0e-7) return std::nullopt;
    const double transferSemiMajor = 0.5 * (radius + targetOppositeApsisRadiusKm);
    const auto targetSpeed = visVivaVelocityKmPerSec(mu, radius, transferSemiMajor);
    if (!targetSpeed) return std::nullopt;
    return *targetSpeed - glm::dot(state.velocityKmPerSec, frame->tangential);
}

} // namespace solar::astrodynamics
