#include "astrodynamics/OrbitalMechanics.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/geometric.hpp>

#include <cmath>

namespace {
constexpr double kEarthMu = 398600.4418;
}

TEST_CASE("Core orbital equations agree for a circular orbit", "[astrodynamics]")
{
    constexpr double radius = 6578.137;
    const double expectedSpeed = std::sqrt(kEarthMu / radius);
    const solar::astronomy::StateVector state{
        {radius, 0.0, 0.0}, {0.0, expectedSpeed, 0.0}};

    REQUIRE(solar::astrodynamics::circularVelocityKmPerSec(kEarthMu, radius));
    CHECK(*solar::astrodynamics::circularVelocityKmPerSec(kEarthMu, radius) ==
          Catch::Approx(expectedSpeed).epsilon(1.0e-12));
    CHECK(*solar::astrodynamics::escapeVelocityKmPerSec(kEarthMu, radius) ==
          Catch::Approx(std::sqrt(2.0) * expectedSpeed).epsilon(1.0e-12));
    CHECK(*solar::astrodynamics::visVivaVelocityKmPerSec(kEarthMu, radius, radius) ==
          Catch::Approx(expectedSpeed).epsilon(1.0e-12));
    CHECK(*solar::astrodynamics::specificOrbitalEnergy(state, kEarthMu) ==
          Catch::Approx(-kEarthMu / (2.0 * radius)).epsilon(1.0e-12));
    CHECK(glm::length(solar::astrodynamics::specificAngularMomentumVector(state)) ==
          Catch::Approx(radius * expectedSpeed).epsilon(1.0e-12));
    CHECK(glm::length(*solar::astrodynamics::eccentricityVector(state, kEarthMu)) ==
          Catch::Approx(0.0).margin(1.0e-12));
}

TEST_CASE("Invalid orbital equation inputs are controlled", "[astrodynamics]")
{
    CHECK_FALSE(solar::astrodynamics::circularVelocityKmPerSec(-1.0, 7000.0));
    CHECK_FALSE(solar::astrodynamics::escapeVelocityKmPerSec(kEarthMu, 0.0));
    CHECK_FALSE(solar::astrodynamics::visVivaVelocityKmPerSec(kEarthMu, 7000.0, 1000.0));
}
