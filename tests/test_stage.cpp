#include "RocketFixtures.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Stage dry mass includes each engine exactly once", "[rocket][stage][mass]")
{
    auto stage = solar::test::testStage("Cluster Stage", 10'000.0, 50'000.0, 4);
    stage.otherDryMassKg = 500.0;
    stage.interstageMassKg = 250.0;
    CHECK(stage.engineClusterMassKg() == 4'000.0);
    CHECK(stage.dryMassKg() == 14'750.0);
    CHECK(stage.wetMassKg() == 64'750.0);
    CHECK(stage.structuralCoefficient() == Catch::Approx(14'750.0 / 64'750.0));
    CHECK(stage.propellantFraction() == Catch::Approx(50'000.0 / 64'750.0));
}

TEST_CASE("Stage rejects invalid engine count and mass", "[rocket][stage][validation]")
{
    auto stage = solar::test::testStage("Invalid", -1.0, 100.0, 0);
    const auto validation = solar::rocket::validateStage(stage);
    CHECK_FALSE(validation.valid());
    CHECK(validation.errors.size() >= 2);
}
