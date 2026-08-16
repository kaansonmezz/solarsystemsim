#include "astrodynamics/OrbitPropagation.hpp"

#include "astrodynamics/Kepler.hpp"
#include "astrodynamics/OrbitConversions.hpp"
#include "astrodynamics/OrbitalMechanics.hpp"

#include <cmath>

namespace solar::astrodynamics {

std::optional<astronomy::StateVector> propagateTwoBody(
    const astronomy::StateVector& stateAtEpoch,
    const double mu,
    const double deltaTimeSeconds) noexcept
{
    if (!std::isfinite(deltaTimeSeconds)) return std::nullopt;
    auto elements = stateToOrbitalElements(stateAtEpoch, mu);
    if (!elements) return std::nullopt;
    if (elements->type() == OrbitType::parabolic) return std::nullopt;
    const auto motion = meanMotionRadPerSec(mu, elements->semiMajorAxisKm);
    if (!motion) return std::nullopt;

    if (elements->eccentricity < 1.0) {
        const auto initialEccentric = trueToEccentricAnomaly(
            elements->trueAnomalyRad, elements->eccentricity);
        if (!initialEccentric) return std::nullopt;
        const double initialMean = ellipticMeanAnomaly(*initialEccentric, elements->eccentricity);
        const auto solved = solveEllipticKepler(
            initialMean + *motion * deltaTimeSeconds, elements->eccentricity);
        if (!solved) return std::nullopt;
        const auto trueAnomaly = eccentricToTrueAnomaly(
            solved->anomalyRad, elements->eccentricity);
        if (!trueAnomaly) return std::nullopt;
        elements->trueAnomalyRad = *trueAnomaly;
    } else if (elements->eccentricity > 1.0) {
        const auto initialHyperbolic = trueToHyperbolicAnomaly(
            elements->trueAnomalyRad, elements->eccentricity);
        if (!initialHyperbolic) return std::nullopt;
        const double initialMean = hyperbolicMeanAnomaly(
            *initialHyperbolic, elements->eccentricity);
        const auto solved = solveHyperbolicKepler(
            initialMean + *motion * deltaTimeSeconds, elements->eccentricity);
        if (!solved) return std::nullopt;
        const auto trueAnomaly = hyperbolicToTrueAnomaly(
            solved->anomalyRad, elements->eccentricity);
        if (!trueAnomaly) return std::nullopt;
        elements->trueAnomalyRad = *trueAnomaly;
    } else {
        return std::nullopt;
    }
    return orbitalElementsToState(*elements, mu);
}

} // namespace solar::astrodynamics
