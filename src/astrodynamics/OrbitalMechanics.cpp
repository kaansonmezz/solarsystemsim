#include "astrodynamics/OrbitalMechanics.hpp"

#include "astrodynamics/Tolerances.hpp"

#include <glm/geometric.hpp>

#include <cmath>

namespace solar::astrodynamics {

std::optional<double> circularVelocityKmPerSec(
    const double mu, const double radius) noexcept
{
    if (!(mu > 0.0) || !(radius > kVectorTolerance)) return std::nullopt;
    return std::sqrt(mu / radius);
}

std::optional<double> escapeVelocityKmPerSec(
    const double mu, const double radius) noexcept
{
    if (!(mu > 0.0) || !(radius > kVectorTolerance)) return std::nullopt;
    return std::sqrt(2.0 * mu / radius);
}

std::optional<double> visVivaVelocityKmPerSec(
    const double mu, const double radius, const double semiMajorAxis) noexcept
{
    if (!(mu > 0.0) || !(radius > kVectorTolerance) ||
        std::abs(semiMajorAxis) <= kVectorTolerance) return std::nullopt;
    const double squaredVelocity = mu * (2.0 / radius - 1.0 / semiMajorAxis);
    if (squaredVelocity < 0.0 || !std::isfinite(squaredVelocity)) return std::nullopt;
    return std::sqrt(squaredVelocity);
}

std::optional<double> specificOrbitalEnergy(
    const astronomy::StateVector& state, const double mu) noexcept
{
    const double radius = glm::length(state.positionKm);
    if (!(mu > 0.0) || !(radius > kVectorTolerance)) return std::nullopt;
    return 0.5 * glm::dot(state.velocityKmPerSec, state.velocityKmPerSec) - mu / radius;
}

glm::dvec3 specificAngularMomentumVector(const astronomy::StateVector& state) noexcept
{
    return glm::cross(state.positionKm, state.velocityKmPerSec);
}

std::optional<glm::dvec3> eccentricityVector(
    const astronomy::StateVector& state, const double mu) noexcept
{
    const double radius = glm::length(state.positionKm);
    if (!(mu > 0.0) || !(radius > kVectorTolerance)) return std::nullopt;
    const glm::dvec3 h = specificAngularMomentumVector(state);
    if (glm::length(h) <= kVectorTolerance) return std::nullopt;
    return glm::cross(state.velocityKmPerSec, h) / mu - state.positionKm / radius;
}

std::optional<double> meanMotionRadPerSec(
    const double mu, const double semiMajorAxis) noexcept
{
    if (!(mu > 0.0) || !std::isfinite(semiMajorAxis) ||
        std::abs(semiMajorAxis) <= kVectorTolerance) return std::nullopt;
    return std::sqrt(mu / std::pow(std::abs(semiMajorAxis), 3.0));
}

} // namespace solar::astrodynamics
