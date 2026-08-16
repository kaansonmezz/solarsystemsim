#pragma once

#include "astronomy/StateVector.hpp"

#include <optional>

#include <glm/vec3.hpp>

namespace solar::astrodynamics {

[[nodiscard]] std::optional<double> circularVelocityKmPerSec(double muKm3PerSec2,
                                                             double radiusKm) noexcept;
[[nodiscard]] std::optional<double> escapeVelocityKmPerSec(double muKm3PerSec2,
                                                           double radiusKm) noexcept;
[[nodiscard]] std::optional<double> visVivaVelocityKmPerSec(double muKm3PerSec2,
                                                            double radiusKm,
                                                            double semiMajorAxisKm) noexcept;
[[nodiscard]] std::optional<double> specificOrbitalEnergy(
    const astronomy::StateVector& state,
    double muKm3PerSec2) noexcept;
[[nodiscard]] glm::dvec3 specificAngularMomentumVector(
    const astronomy::StateVector& state) noexcept;
[[nodiscard]] std::optional<glm::dvec3> eccentricityVector(
    const astronomy::StateVector& state,
    double muKm3PerSec2) noexcept;
[[nodiscard]] std::optional<double> meanMotionRadPerSec(double muKm3PerSec2,
                                                        double semiMajorAxisKm) noexcept;

} // namespace solar::astrodynamics

