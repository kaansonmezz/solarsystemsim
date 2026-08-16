#include "astrodynamics/Angles.hpp"
#include "astrodynamics/OrbitConversions.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/geometric.hpp>

#include <numbers>
#include <array>

namespace {
constexpr double kEarthMu = 398600.4418;

void checkStateNear(const solar::astronomy::StateVector& actual,
                    const solar::astronomy::StateVector& expected)
{
    CHECK(glm::length(actual.positionKm - expected.positionKm) ==
          Catch::Approx(0.0).margin(1.0e-7));
    CHECK(glm::length(actual.velocityKmPerSec - expected.velocityKmPerSec) ==
          Catch::Approx(0.0).margin(1.0e-10));
}
}

TEST_CASE("Inclined elliptic element and state conversions round trip", "[orbit-conversion]")
{
    const solar::astrodynamics::OrbitalElements source{
        12000.0, 0.35, 0.73, 1.1, 0.42, 2.0, 12000.0 * (1.0 - 0.35 * 0.35)};
    const auto state = solar::astrodynamics::orbitalElementsToState(source, kEarthMu);
    REQUIRE(state);
    const auto recovered = solar::astrodynamics::stateToOrbitalElements(*state, kEarthMu);
    REQUIRE(recovered);
    CHECK(recovered->semiMajorAxisKm == Catch::Approx(source.semiMajorAxisKm).epsilon(1.0e-11));
    CHECK(recovered->eccentricity == Catch::Approx(source.eccentricity).epsilon(1.0e-11));
    CHECK(recovered->inclinationRad == Catch::Approx(source.inclinationRad).margin(1.0e-11));
    CHECK(solar::astrodynamics::angularDifference(
              recovered->longitudeAscendingNodeRad, source.longitudeAscendingNodeRad) ==
          Catch::Approx(0.0).margin(1.0e-11));
    CHECK(solar::astrodynamics::angularDifference(
              recovered->argumentPeriapsisRad, source.argumentPeriapsisRad) ==
          Catch::Approx(0.0).margin(1.0e-11));
    CHECK(solar::astrodynamics::angularDifference(
              recovered->trueAnomalyRad, source.trueAnomalyRad) ==
          Catch::Approx(0.0).margin(1.0e-11));
    checkStateNear(*solar::astrodynamics::orbitalElementsToState(*recovered, kEarthMu), *state);
}

TEST_CASE("Circular equatorial convention remains finite", "[orbit-conversion]")
{
    const double radius = 7000.0;
    const solar::astronomy::StateVector state{
        {0.0, radius, 0.0}, {-std::sqrt(kEarthMu / radius), 0.0, 0.0}};
    const auto elements = solar::astrodynamics::stateToOrbitalElements(state, kEarthMu);
    REQUIRE(elements);
    CHECK(elements->type() == solar::astrodynamics::OrbitType::circular);
    CHECK(elements->longitudeAscendingNodeRad == 0.0);
    CHECK(elements->argumentPeriapsisRad == 0.0);
    REQUIRE(solar::astrodynamics::orbitalElementsToState(*elements, kEarthMu));
    checkStateNear(*solar::astrodynamics::orbitalElementsToState(*elements, kEarthMu), state);
}

TEST_CASE("Hyperbolic state is classified without fictitious period", "[orbit-conversion]")
{
    const solar::astronomy::StateVector state{{7000.0, 0.0, 0.0}, {0.0, 12.0, 2.0}};
    const auto elements = solar::astrodynamics::stateToOrbitalElements(state, kEarthMu);
    REQUIRE(elements);
    CHECK(elements->type() == solar::astrodynamics::OrbitType::hyperbolic);
    CHECK(elements->eccentricity > 1.0);
    CHECK(elements->semiMajorAxisKm < 0.0);
    CHECK_FALSE(elements->apoapsisRadiusKm());
    CHECK_FALSE(elements->periodSeconds(kEarthMu));
    checkStateNear(*solar::astrodynamics::orbitalElementsToState(*elements, kEarthMu), state);
}

TEST_CASE("Prograde equatorial eccentric orbit preserves its state", "[orbit-conversion][regression]")
{
    const solar::astrodynamics::OrbitalElements source{
        10000.0, 0.2, 0.0, 0.0, 1.2, 2.0, 9600.0};
    const auto state = solar::astrodynamics::orbitalElementsToState(source, kEarthMu);
    REQUIRE(state);
    const auto recovered = solar::astrodynamics::stateToOrbitalElements(*state, kEarthMu);
    REQUIRE(recovered);
    REQUIRE(recovered->longitudeAscendingNodeRad == 0.0);
    const auto reconstructed = solar::astrodynamics::orbitalElementsToState(*recovered, kEarthMu);
    REQUIRE(reconstructed);
    checkStateNear(*reconstructed, *state);
}

TEST_CASE("Retrograde equatorial eccentric orbit preserves its state", "[orbit-conversion][regression]")
{
    const solar::astrodynamics::OrbitalElements source{
        10000.0, 0.2, std::numbers::pi, 0.0, 1.2, 2.0, 9600.0};
    const auto state = solar::astrodynamics::orbitalElementsToState(source, kEarthMu);
    REQUIRE(state);
    const auto recovered = solar::astrodynamics::stateToOrbitalElements(*state, kEarthMu);
    REQUIRE(recovered);
    const auto reconstructed = solar::astrodynamics::orbitalElementsToState(*recovered, kEarthMu);
    REQUIRE(reconstructed);
    checkStateNear(*reconstructed, *state);
}

TEST_CASE("Retrograde equatorial state round trips across angle boundaries",
          "[orbit-conversion][regression]")
{
    constexpr std::array<double, 5> arguments{0.0, 1.0e-12, 1.2,
                                               std::numbers::pi, 2.0 * std::numbers::pi - 1.0e-12};
    constexpr std::array<double, 5> anomalies{0.0, 1.0e-12, 2.0,
                                               std::numbers::pi + 0.1,
                                               2.0 * std::numbers::pi - 1.0e-12};
    for (const double argument : arguments) {
        for (const double anomaly : anomalies) {
            CAPTURE(argument, anomaly);
            const solar::astrodynamics::OrbitalElements source{
                10000.0, 0.2, std::numbers::pi, 0.0, argument, anomaly, 9600.0};
            const auto state = solar::astrodynamics::orbitalElementsToState(source, kEarthMu);
            REQUIRE(state);
            const auto recovered = solar::astrodynamics::stateToOrbitalElements(*state, kEarthMu);
            REQUIRE(recovered);
            const auto reconstructed = solar::astrodynamics::orbitalElementsToState(
                *recovered, kEarthMu);
            REQUIRE(reconstructed);
            checkStateNear(*reconstructed, *state);
        }
    }
}
