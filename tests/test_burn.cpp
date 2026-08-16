#include "rocket/Burn.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Constant mass flow determines burn duration", "[rocket][burn]")
{
    REQUIRE(solar::rocket::burnTimeSeconds(1'000.0, 20.0));
    CHECK(*solar::rocket::burnTimeSeconds(1'000.0, 20.0) == 50.0);
    REQUIRE(solar::rocket::burnTimeSeconds(0.0, 0.0));
    CHECK(*solar::rocket::burnTimeSeconds(0.0, 0.0) == 0.0);
    CHECK_FALSE(solar::rocket::burnTimeSeconds(1'000.0, 0.0));
    CHECK_FALSE(solar::rocket::burnTimeSeconds(-1.0, 20.0));
}
