#include "astrodynamics/PlaneChange.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <numbers>

TEST_CASE("Pure plane-change calculator implements the ideal impulsive equation", "[plane-change]")
{
    const auto deltaVelocity = solar::astrodynamics::planeChangeDeltaVelocityKmPerSec(
        7.5, 30.0 * std::numbers::pi / 180.0);
    REQUIRE(deltaVelocity);
    CHECK(*deltaVelocity == Catch::Approx(2.0 * 7.5 * std::sin(15.0 * std::numbers::pi / 180.0)));
    CHECK_FALSE(solar::astrodynamics::planeChangeDeltaVelocityKmPerSec(-1.0, 0.1));
}
