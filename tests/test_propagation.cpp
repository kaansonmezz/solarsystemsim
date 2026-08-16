#include "astrodynamics/OrbitConversions.hpp"
#include "astrodynamics/OrbitPropagation.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/geometric.hpp>

#include <numbers>

namespace {
constexpr double kEarthMu = 398600.4418;
const solar::astrodynamics::OrbitalElements kElements{
    10000.0, 0.2, 0.5, 0.8, 1.2, 0.3, 9600.0};
}

TEST_CASE("One elliptic period returns to the initial state", "[propagation]")
{
    const auto initial = solar::astrodynamics::orbitalElementsToState(kElements, kEarthMu);
    REQUIRE(initial);
    const double period = 2.0 * std::numbers::pi *
        std::sqrt(10000.0 * 10000.0 * 10000.0 / kEarthMu);
    const auto final = solar::astrodynamics::propagateTwoBody(*initial, kEarthMu, period);
    REQUIRE(final);
    CHECK(glm::length(final->positionKm - initial->positionKm) ==
          Catch::Approx(0.0).margin(1.0e-6));
    CHECK(glm::length(final->velocityKmPerSec - initial->velocityKmPerSec) ==
          Catch::Approx(0.0).margin(1.0e-9));
}

TEST_CASE("Analytical propagation is reversible and conserves invariants", "[propagation]")
{
    const auto initial = solar::astrodynamics::orbitalElementsToState(kElements, kEarthMu);
    REQUIRE(initial);
    const auto forward = solar::astrodynamics::propagateTwoBody(*initial, kEarthMu, 2789.25);
    REQUIRE(forward);
    const auto backward = solar::astrodynamics::propagateTwoBody(*forward, kEarthMu, -2789.25);
    REQUIRE(backward);
    CHECK(glm::length(backward->positionKm - initial->positionKm) ==
          Catch::Approx(0.0).margin(1.0e-6));
    CHECK(glm::length(backward->velocityKmPerSec - initial->velocityKmPerSec) ==
          Catch::Approx(0.0).margin(1.0e-9));
    const auto initialElements = solar::astrodynamics::stateToOrbitalElements(*initial, kEarthMu);
    const auto finalElements = solar::astrodynamics::stateToOrbitalElements(*forward, kEarthMu);
    REQUIRE(initialElements);
    REQUIRE(finalElements);
    CHECK(finalElements->semiMajorAxisKm == Catch::Approx(initialElements->semiMajorAxisKm).epsilon(1.0e-11));
    CHECK(finalElements->eccentricity == Catch::Approx(initialElements->eccentricity).margin(1.0e-11));
    CHECK(finalElements->semiLatusRectumKm == Catch::Approx(initialElements->semiLatusRectumKm).epsilon(1.0e-11));
}

TEST_CASE("Hyperbolic propagation supports negative time", "[propagation]")
{
    const solar::astronomy::StateVector initial{{7000.0, 0.0, 0.0}, {0.0, 11.2, 1.0}};
    const auto forward = solar::astrodynamics::propagateTwoBody(initial, kEarthMu, 600.0);
    REQUIRE(forward);
    const auto backward = solar::astrodynamics::propagateTwoBody(*forward, kEarthMu, -600.0);
    REQUIRE(backward);
    CHECK(glm::length(backward->positionKm - initial.positionKm) ==
          Catch::Approx(0.0).margin(1.0e-5));
    CHECK(glm::length(backward->velocityKmPerSec - initial.velocityKmPerSec) ==
          Catch::Approx(0.0).margin(1.0e-8));
}
