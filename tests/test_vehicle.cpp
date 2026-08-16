#include "RocketFixtures.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Vehicle totals distinguish dry propellant payload and liftoff mass", "[rocket][vehicle]")
{
    const auto vehicle = solar::test::twoStageVehicle();
    // Dry includes 2,000 kg and 1,000 kg engine clusters.
    CHECK(vehicle.dryMassKg() == 33'000.0);
    CHECK(vehicle.propellantMassKg() == 120'000.0);
    CHECK(vehicle.liftoffMassKg() == 153'000.0);
    CHECK(vehicle.payloadFraction() == Catch::Approx(5'000.0 / 153'000.0));
}

TEST_CASE("Vehicle stage ordering is chronological", "[rocket][vehicle][staging]")
{
    const auto vehicle = solar::test::twoStageVehicle();
    REQUIRE(vehicle.stages.size() == 2);
    CHECK(vehicle.stages[0].name == "Stage 1");
    CHECK(vehicle.stages[1].name == "Stage 2");
}
