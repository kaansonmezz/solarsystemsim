#include "astrodynamics/Kepler.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

TEST_CASE("Elliptic Kepler solver closes its equation", "[kepler]")
{
    for (const double eccentricity : {0.0, 0.1, 0.65, 0.95, 0.999}) {
        for (const double mean : {-25.0, -1.0, 0.0, 2.4, 30.0}) {
            const auto solution = solar::astrodynamics::solveEllipticKepler(mean, eccentricity);
            REQUIRE(solution);
            const double normalizedMean = solar::astrodynamics::ellipticMeanAnomaly(
                solution->anomalyRad, eccentricity);
            CHECK(std::sin(normalizedMean - mean) == Catch::Approx(0.0).margin(1.0e-11));
        }
    }
}

TEST_CASE("Hyperbolic Kepler solver closes its equation", "[kepler]")
{
    for (const double eccentricity : {1.1, 1.5, 3.0}) {
        for (const double mean : {-12.0, -0.2, 0.0, 0.2, 12.0}) {
            const auto solution = solar::astrodynamics::solveHyperbolicKepler(mean, eccentricity);
            REQUIRE(solution);
            CHECK(solar::astrodynamics::hyperbolicMeanAnomaly(
                      solution->anomalyRad, eccentricity) ==
                  Catch::Approx(mean).margin(1.0e-11));
        }
    }
}

TEST_CASE("Anomaly conversions round trip", "[kepler]")
{
    const auto eccentric = solar::astrodynamics::trueToEccentricAnomaly(2.1, 0.7);
    REQUIRE(eccentric);
    CHECK(*solar::astrodynamics::eccentricToTrueAnomaly(*eccentric, 0.7) ==
          Catch::Approx(2.1).margin(1.0e-12));
    const auto hyperbolic = solar::astrodynamics::trueToHyperbolicAnomaly(0.8, 1.7);
    REQUIRE(hyperbolic);
    CHECK(*solar::astrodynamics::hyperbolicToTrueAnomaly(*hyperbolic, 1.7) ==
          Catch::Approx(0.8).margin(1.0e-12));
}
