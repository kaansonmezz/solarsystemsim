#include "astronomy/BodyDatabase.hpp"
#include "rocket/TWR.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("TWR follows thrust over mass and local gravity", "[rocket][twr]")
{
    const auto twr = solar::rocket::thrustToWeightRatio(200'000.0, 10'000.0, 4.0);
    REQUIRE(twr);
    CHECK(*twr == Catch::Approx(5.0));
    CHECK_FALSE(solar::rocket::thrustToWeightRatio(200'000.0, 0.0, 4.0));
}

TEST_CASE("Body-derived TWR is Moon then Mars then Earth", "[rocket][twr][astronomy]")
{
    const solar::astronomy::BodyDatabase bodies;
    const auto* earth = bodies.find("Earth");
    const auto* moon = bodies.find("Moon");
    const auto* mars = bodies.find("Mars");
    REQUIRE(earth);
    REQUIRE(moon);
    REQUIRE(mars);
    const auto earthTwr = solar::rocket::thrustToWeightRatio(100'000.0, 10'000.0, *earth);
    const auto moonTwr = solar::rocket::thrustToWeightRatio(100'000.0, 10'000.0, *moon);
    const auto marsTwr = solar::rocket::thrustToWeightRatio(100'000.0, 10'000.0, *mars);
    REQUIRE(earthTwr);
    REQUIRE(moonTwr);
    REQUIRE(marsTwr);
    CHECK(*moonTwr > *marsTwr);
    CHECK(*marsTwr > *earthTwr);
}
