#include "rocket/DeltaV.hpp"
#include "rocket/Units.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

TEST_CASE("Tsiolkovsky equation uses standard gravity", "[rocket][delta-v]")
{
    const auto deltaV = solar::rocket::idealDeltaVMetersPerSec(100'000.0, 40'000.0, 300.0);
    REQUIRE(deltaV);
    const double expected = 300.0 * 9.80665 * std::log(2.5);
    CHECK(*deltaV == Catch::Approx(expected).epsilon(1.0e-13));
    CHECK(*solar::rocket::effectiveExhaustVelocityMetersPerSec(300.0) ==
          Catch::Approx(2'941.995));
}

TEST_CASE("Zero propellant yields zero ideal delta-v", "[rocket][delta-v]")
{
    REQUIRE(solar::rocket::idealDeltaVMetersPerSec(10'000.0, 10'000.0, 300.0));
    CHECK(*solar::rocket::idealDeltaVMetersPerSec(10'000.0, 10'000.0, 300.0) == 0.0);
    CHECK_FALSE(solar::rocket::idealDeltaVMetersPerSec(10'000.0, 0.0, 300.0));
    CHECK_FALSE(solar::rocket::idealDeltaVMetersPerSec(9'000.0, 10'000.0, 300.0));
}

TEST_CASE("Required propellant recovers requested delta-v", "[rocket][delta-v][partial]")
{
    constexpr double initialMass = 20'000.0;
    constexpr double requested = 1'500.0;
    constexpr double isp = 340.0;
    const auto used = solar::rocket::requiredPropellantMassKg(initialMass, requested, isp);
    REQUIRE(used);
    const auto recovered = solar::rocket::idealDeltaVMetersPerSec(
        initialMass, initialMass - *used, isp);
    REQUIRE(recovered);
    CHECK(*recovered == Catch::Approx(requested).epsilon(1.0e-12));
    CHECK(*solar::rocket::remainingIdealDeltaVMetersPerSec(initialMass, *used, isp) ==
          Catch::Approx(requested).epsilon(1.0e-12));
}

TEST_CASE("Rocket and astrodynamics velocity conversion is explicit", "[rocket][units]")
{
    CHECK(solar::rocket::metersPerSecToKilometersPerSec(3'250.0) == 3.25);
    CHECK(solar::rocket::kilometersPerSecToMetersPerSec(3.25) == 3'250.0);
}
