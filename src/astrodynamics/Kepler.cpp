#include "astrodynamics/Kepler.hpp"

#include "astrodynamics/Angles.hpp"
#include "astrodynamics/Tolerances.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace solar::astrodynamics {

std::optional<KeplerSolution> solveEllipticKepler(
    const double meanAnomalyRad, const double eccentricity) noexcept
{
    if (!std::isfinite(meanAnomalyRad) || eccentricity < 0.0 || eccentricity >= 1.0) {
        return std::nullopt;
    }
    const double mean = normalizeAngleSigned(meanAnomalyRad);
    double eccentric = eccentricity < 0.8 ? mean : std::copysign(std::numbers::pi, mean);
    for (int iteration = 1; iteration <= kKeplerMaximumIterations; ++iteration) {
        const double residual = eccentric - eccentricity * std::sin(eccentric) - mean;
        const double derivative = 1.0 - eccentricity * std::cos(eccentric);
        if (std::abs(derivative) <= kVectorTolerance) return std::nullopt;
        const double correction = residual / derivative;
        eccentric -= correction;
        if (std::abs(correction) <= kKeplerTolerance) {
            return KeplerSolution{eccentric, iteration};
        }
    }
    return std::nullopt;
}

std::optional<KeplerSolution> solveHyperbolicKepler(
    const double meanAnomalyRad, const double eccentricity) noexcept
{
    if (!std::isfinite(meanAnomalyRad) || !(eccentricity > 1.0)) return std::nullopt;
    double hyperbolic = std::asinh(meanAnomalyRad / eccentricity);
    for (int iteration = 1; iteration <= kKeplerMaximumIterations; ++iteration) {
        const double residual = eccentricity * std::sinh(hyperbolic) - hyperbolic -
                                meanAnomalyRad;
        const double derivative = eccentricity * std::cosh(hyperbolic) - 1.0;
        if (std::abs(derivative) <= kVectorTolerance) return std::nullopt;
        const double correction = residual / derivative;
        hyperbolic -= correction;
        if (std::abs(correction) <= kKeplerTolerance) {
            return KeplerSolution{hyperbolic, iteration};
        }
    }
    return std::nullopt;
}

std::optional<double> trueToEccentricAnomaly(
    const double trueAnomalyRad, const double eccentricity) noexcept
{
    if (!std::isfinite(trueAnomalyRad) || eccentricity < 0.0 || eccentricity >= 1.0) {
        return std::nullopt;
    }
    const double y = std::sqrt(1.0 - eccentricity) * std::sin(trueAnomalyRad / 2.0);
    const double x = std::sqrt(1.0 + eccentricity) * std::cos(trueAnomalyRad / 2.0);
    return normalizeAnglePositive(2.0 * std::atan2(y, x));
}

std::optional<double> eccentricToTrueAnomaly(
    const double eccentricAnomalyRad, const double eccentricity) noexcept
{
    if (!std::isfinite(eccentricAnomalyRad) || eccentricity < 0.0 || eccentricity >= 1.0) {
        return std::nullopt;
    }
    const double y = std::sqrt(1.0 + eccentricity) * std::sin(eccentricAnomalyRad / 2.0);
    const double x = std::sqrt(1.0 - eccentricity) * std::cos(eccentricAnomalyRad / 2.0);
    return normalizeAnglePositive(2.0 * std::atan2(y, x));
}

std::optional<double> trueToHyperbolicAnomaly(
    const double trueAnomalyRad, const double eccentricity) noexcept
{
    if (!std::isfinite(trueAnomalyRad) || !(eccentricity > 1.0)) return std::nullopt;
    const double argument = std::sqrt((eccentricity - 1.0) / (eccentricity + 1.0)) *
                            std::tan(trueAnomalyRad / 2.0);
    if (!(std::abs(argument) < 1.0)) return std::nullopt;
    return 2.0 * std::atanh(argument);
}

std::optional<double> hyperbolicToTrueAnomaly(
    const double hyperbolicAnomalyRad, const double eccentricity) noexcept
{
    if (!std::isfinite(hyperbolicAnomalyRad) || !(eccentricity > 1.0)) return std::nullopt;
    return 2.0 * std::atan2(
        std::sqrt(eccentricity + 1.0) * std::sinh(hyperbolicAnomalyRad / 2.0),
        std::sqrt(eccentricity - 1.0) * std::cosh(hyperbolicAnomalyRad / 2.0));
}

double ellipticMeanAnomaly(const double eccentricAnomalyRad,
                           const double eccentricity) noexcept
{
    return eccentricAnomalyRad - eccentricity * std::sin(eccentricAnomalyRad);
}

double hyperbolicMeanAnomaly(const double hyperbolicAnomalyRad,
                             const double eccentricity) noexcept
{
    return eccentricity * std::sinh(hyperbolicAnomalyRad) - hyperbolicAnomalyRad;
}

} // namespace solar::astrodynamics
