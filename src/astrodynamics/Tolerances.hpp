#pragma once

namespace solar::astrodynamics {

inline constexpr double kVectorTolerance = 1.0e-12;
inline constexpr double kCircularEccentricityTolerance = 1.0e-10;
inline constexpr double kEquatorialToleranceRad = 1.0e-10;
inline constexpr double kParabolicEccentricityTolerance = 1.0e-8;
inline constexpr double kKeplerTolerance = 1.0e-13;
inline constexpr int kKeplerMaximumIterations = 64;

} // namespace solar::astrodynamics

