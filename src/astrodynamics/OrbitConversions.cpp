#include "astrodynamics/OrbitConversions.hpp"

#include "astrodynamics/Angles.hpp"
#include "astrodynamics/OrbitalMechanics.hpp"
#include "astrodynamics/Tolerances.hpp"

#include <glm/geometric.hpp>
#include <glm/mat3x3.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

namespace solar::astrodynamics {
namespace {

[[nodiscard]] double safeAcos(const double value) noexcept
{
    return std::acos(std::clamp(value, -1.0, 1.0));
}

[[nodiscard]] double orientedAngle(
    const glm::dvec3& from,
    const glm::dvec3& to,
    const glm::dvec3& positiveNormal) noexcept
{
    const double denominator = glm::length(from) * glm::length(to);
    if (denominator <= kVectorTolerance) return 0.0;
    const double cosine = std::clamp(glm::dot(from, to) / denominator, -1.0, 1.0);
    const double sine = glm::dot(glm::cross(from, to), positiveNormal) /
                        (denominator * glm::length(positiveNormal));
    return normalizeAnglePositive(std::atan2(sine, cosine));
}

} // namespace

std::optional<OrbitalElements> stateToOrbitalElements(
    const astronomy::StateVector& state, const double mu) noexcept
{
    if (!(mu > 0.0) || !astronomy::isFinite(state)) return std::nullopt;
    const glm::dvec3& r = state.positionKm;
    const glm::dvec3& v = state.velocityKmPerSec;
    const double radius = glm::length(r);
    if (!(radius > kVectorTolerance)) return std::nullopt;

    const glm::dvec3 h = glm::cross(r, v);
    const double hMagnitude = glm::length(h);
    if (!(hMagnitude > kVectorTolerance)) return std::nullopt;
    const glm::dvec3 node = glm::cross(glm::dvec3{0.0, 0.0, 1.0}, h);
    const double nodeMagnitude = glm::length(node);
    const auto eccentricityVectorValue = eccentricityVector(state, mu);
    const auto energyValue = specificOrbitalEnergy(state, mu);
    if (!eccentricityVectorValue || !energyValue) return std::nullopt;
    const glm::dvec3 eVector = *eccentricityVectorValue;
    const double eccentricity = glm::length(eVector);
    const double inclination = safeAcos(h.z / hMagnitude);
    const bool circular = eccentricity <= kCircularEccentricityTolerance;
    const bool equatorial = inclination <= kEquatorialToleranceRad ||
                            std::abs(inclination - std::numbers::pi) <= kEquatorialToleranceRad;

    double raan = 0.0;
    if (!equatorial && nodeMagnitude > kVectorTolerance) {
        raan = normalizeAnglePositive(std::atan2(node.y, node.x));
    }

    double argumentPeriapsis = 0.0;
    double trueAnomaly = 0.0;
    if (!circular && !equatorial) {
        argumentPeriapsis = orientedAngle(node, eVector, h);
        trueAnomaly = orientedAngle(eVector, r, h);
    } else if (!circular) {
        // With RAAN fixed to zero, argumentPeriapsis stores longitude of periapsis.
        // R1(pi) mirrors inertial Y for a retrograde equatorial orbit, so the inverse
        // mapping must follow the angular-momentum direction.
        const double inertialLongitude = std::atan2(eVector.y, eVector.x);
        argumentPeriapsis = normalizeAnglePositive(
            h.z >= 0.0 ? inertialLongitude : -inertialLongitude);
        trueAnomaly = orientedAngle(eVector, r, h);
    } else if (!equatorial) {
        trueAnomaly = orientedAngle(node, r, h);
    } else {
        trueAnomaly = normalizeAnglePositive(std::atan2(r.y, r.x));
        if (h.z < 0.0) trueAnomaly = normalizeAnglePositive(-trueAnomaly);
    }

    double semiMajorAxis = std::numeric_limits<double>::infinity();
    if (std::abs(*energyValue) > kVectorTolerance) {
        semiMajorAxis = -mu / (2.0 * *energyValue);
    }
    const double semiLatusRectum = hMagnitude * hMagnitude / mu;
    OrbitalElements elements{
        semiMajorAxis,
        eccentricity,
        inclination,
        raan,
        argumentPeriapsis,
        trueAnomaly,
        semiLatusRectum,
    };
    return elements.type() == OrbitType::degenerate ? std::nullopt
                                                     : std::optional<OrbitalElements>{elements};
}

std::optional<astronomy::StateVector> orbitalElementsToState(
    const OrbitalElements& elements, const double mu) noexcept
{
    if (!(mu > 0.0) || elements.eccentricity < 0.0 ||
        !std::isfinite(elements.eccentricity)) return std::nullopt;
    double p = elements.semiLatusRectumKm;
    if (!(p > kVectorTolerance)) {
        if (!std::isfinite(elements.semiMajorAxisKm)) return std::nullopt;
        p = elements.semiMajorAxisKm *
            (1.0 - elements.eccentricity * elements.eccentricity);
    }
    if (!(p > kVectorTolerance)) return std::nullopt;
    const double denominator = 1.0 + elements.eccentricity * std::cos(elements.trueAnomalyRad);
    if (!(denominator > kVectorTolerance)) return std::nullopt;

    const double radius = p / denominator;
    const double cosineTrue = std::cos(elements.trueAnomalyRad);
    const double sineTrue = std::sin(elements.trueAnomalyRad);
    const glm::dvec3 positionPqw{radius * cosineTrue, radius * sineTrue, 0.0};
    const double velocityFactor = std::sqrt(mu / p);
    const glm::dvec3 velocityPqw{
        -velocityFactor * sineTrue,
        velocityFactor * (elements.eccentricity + cosineTrue),
        0.0,
    };

    const double cosineRaan = std::cos(elements.longitudeAscendingNodeRad);
    const double sineRaan = std::sin(elements.longitudeAscendingNodeRad);
    const double cosineInclination = std::cos(elements.inclinationRad);
    const double sineInclination = std::sin(elements.inclinationRad);
    const double cosinePeriapsis = std::cos(elements.argumentPeriapsisRad);
    const double sinePeriapsis = std::sin(elements.argumentPeriapsisRad);

    const glm::dmat3 rotation{
        {cosineRaan * cosinePeriapsis - sineRaan * sinePeriapsis * cosineInclination,
         sineRaan * cosinePeriapsis + cosineRaan * sinePeriapsis * cosineInclination,
         sinePeriapsis * sineInclination},
        {-cosineRaan * sinePeriapsis - sineRaan * cosinePeriapsis * cosineInclination,
         -sineRaan * sinePeriapsis + cosineRaan * cosinePeriapsis * cosineInclination,
         cosinePeriapsis * sineInclination},
        {sineRaan * sineInclination, -cosineRaan * sineInclination, cosineInclination},
    };

    const astronomy::StateVector state{rotation * positionPqw, rotation * velocityPqw};
    return astronomy::isFinite(state) ? std::optional<astronomy::StateVector>{state} : std::nullopt;
}

} // namespace solar::astrodynamics
