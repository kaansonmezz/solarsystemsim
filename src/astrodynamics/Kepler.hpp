#pragma once

#include <optional>

namespace solar::astrodynamics {

struct KeplerSolution final {
    double anomalyRad{};
    int iterations{};
};

[[nodiscard]] std::optional<KeplerSolution> solveEllipticKepler(
    double meanAnomalyRad, double eccentricity) noexcept;
[[nodiscard]] std::optional<KeplerSolution> solveHyperbolicKepler(
    double meanAnomalyRad, double eccentricity) noexcept;

[[nodiscard]] std::optional<double> trueToEccentricAnomaly(
    double trueAnomalyRad, double eccentricity) noexcept;
[[nodiscard]] std::optional<double> eccentricToTrueAnomaly(
    double eccentricAnomalyRad, double eccentricity) noexcept;
[[nodiscard]] std::optional<double> trueToHyperbolicAnomaly(
    double trueAnomalyRad, double eccentricity) noexcept;
[[nodiscard]] std::optional<double> hyperbolicToTrueAnomaly(
    double hyperbolicAnomalyRad, double eccentricity) noexcept;

[[nodiscard]] double ellipticMeanAnomaly(double eccentricAnomalyRad,
                                         double eccentricity) noexcept;
[[nodiscard]] double hyperbolicMeanAnomaly(double hyperbolicAnomalyRad,
                                           double eccentricity) noexcept;

} // namespace solar::astrodynamics
