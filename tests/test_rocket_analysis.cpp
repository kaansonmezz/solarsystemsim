#include "RocketFixtures.hpp"

#include "rocket/EngineDatabase.hpp"
#include "rocket/RocketAnalysis.hpp"
#include "rocket/VehicleDatabase.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

TEST_CASE("Two-stage analysis evolves stack mass across separation", "[rocket][analysis][staging]")
{
    const auto analysis = solar::rocket::analyzeVehicle(
        solar::test::twoStageVehicle(), solar::rocket::PerformanceEnvironment::vacuum, 9.80665);
    REQUIRE(analysis.valid());
    REQUIRE(analysis.stages.size() == 2);
    const auto& first = analysis.stages[0];
    const auto& second = analysis.stages[1];
    CHECK(first.ignitionMassKg == 153'000.0);
    CHECK(first.burnoutMassKg == 53'000.0);
    CHECK(first.postSeparationMassKg == 31'000.0);
    CHECK(second.ignitionMassKg == 31'000.0);
    CHECK(second.burnoutMassKg == 11'000.0);
    CHECK(second.postSeparationMassKg == 5'000.0);
    REQUIRE(first.initialTwr);
    REQUIRE(first.finalTwr);
    CHECK(*first.finalTwr > *first.initialTwr);
    REQUIRE(first.burnTimeSeconds);
    CHECK(*first.burnTimeSeconds == Catch::Approx(
        100'000.0 / (2'000'000.0 / (320.0 * 9.80665))));
}

TEST_CASE("Payload reduces every stage and total ideal delta-v", "[rocket][analysis][payload]")
{
    const auto empty = solar::rocket::analyzeVehicle(
        solar::test::twoStageVehicle(0.0), solar::rocket::PerformanceEnvironment::vacuum, 9.80665);
    const auto loaded = solar::rocket::analyzeVehicle(
        solar::test::twoStageVehicle(15'000.0), solar::rocket::PerformanceEnvironment::vacuum, 9.80665);
    const auto baseline = solar::rocket::analyzeVehicle(
        solar::test::twoStageVehicle(5'000.0), solar::rocket::PerformanceEnvironment::vacuum, 9.80665);
    REQUIRE(empty.valid());
    REQUIRE(loaded.valid());
    REQUIRE(baseline.valid());
    REQUIRE(empty.stages[0].deltaVMetersPerSec);
    REQUIRE(loaded.stages[0].deltaVMetersPerSec);
    REQUIRE(empty.stages[1].deltaVMetersPerSec);
    REQUIRE(loaded.stages[1].deltaVMetersPerSec);
    CHECK(*loaded.stages[0].deltaVMetersPerSec < *empty.stages[0].deltaVMetersPerSec);
    CHECK(*loaded.stages[1].deltaVMetersPerSec < *empty.stages[1].deltaVMetersPerSec);
    CHECK(loaded.totalIdealDeltaVMetersPerSec < empty.totalIdealDeltaVMetersPerSec);
    CHECK(loaded.totalIdealDeltaVMetersPerSec < baseline.totalIdealDeltaVMetersPerSec);
    CHECK(loaded.liftoffMassKg - empty.liftoffMassKg == 15'000.0);
    CHECK(loaded.liftoffMassKg - baseline.liftoffMassKg == 10'000.0);
    REQUIRE(loaded.stages[0].burnMassRatio);
    REQUIRE(baseline.stages[0].burnMassRatio);
    REQUIRE(loaded.stages[0].initialTwr);
    REQUIRE(baseline.stages[0].initialTwr);
    CHECK(*loaded.stages[0].burnMassRatio < *baseline.stages[0].burnMassRatio);
    CHECK(*loaded.stages[0].initialTwr < *baseline.stages[0].initialTwr);
}

TEST_CASE("Vacuum and sea-level reference analyses remain distinct", "[rocket][analysis][environment]")
{
    const auto vehicle = solar::test::twoStageVehicle();
    const auto vacuum = solar::rocket::analyzeVehicle(
        vehicle, solar::rocket::PerformanceEnvironment::vacuum, 9.80665);
    const auto seaLevel = solar::rocket::analyzeVehicle(
        vehicle, solar::rocket::PerformanceEnvironment::seaLevel, 9.80665);
    CHECK(vacuum.totalIdealDeltaVMetersPerSec > seaLevel.totalIdealDeltaVMetersPerSec);
    REQUIRE(vacuum.stages[0].thrustN);
    REQUIRE(seaLevel.stages[0].thrustN);
    CHECK(*vacuum.stages[0].thrustN > *seaLevel.stages[0].thrustN);
}

TEST_CASE("Zero propellant is a safe zero-duration zero-delta-v stage", "[rocket][analysis]")
{
    auto vehicle = solar::test::twoStageVehicle();
    vehicle.stages.resize(1);
    vehicle.stages[0].propellant.massKg = 0.0;
    const auto analysis = solar::rocket::analyzeVehicle(
        vehicle, solar::rocket::PerformanceEnvironment::vacuum, 9.80665);
    REQUIRE(analysis.stages[0].deltaVMetersPerSec);
    REQUIRE(analysis.stages[0].burnTimeSeconds);
    CHECK(*analysis.stages[0].deltaVMetersPerSec == 0.0);
    CHECK(*analysis.stages[0].burnTimeSeconds == 0.0);
}

TEST_CASE("Vehicle and custom engine survive JSON round trip", "[rocket][json]")
{
    const auto original = solar::test::twoStageVehicle(7'500.0);
    const auto loaded = solar::rocket::vehicleFromJson(solar::rocket::vehicleToJson(original));
    REQUIRE(loaded.valid());
    REQUIRE(loaded.vehicle);
    CHECK(loaded.vehicle->name == original.name);
    CHECK(loaded.vehicle->payloadMassKg == original.payloadMassKg);
    REQUIRE(loaded.vehicle->stages.size() == original.stages.size());
    CHECK(loaded.vehicle->stages[0].engine.name == original.stages[0].engine.name);
    CHECK(loaded.vehicle->stages[0].engineCount == original.stages[0].engineCount);
    CHECK(loaded.vehicle->liftoffMassKg() == original.liftoffMassKg());

    std::vector<std::string> errors;
    const auto engine = solar::rocket::engineFromJson(
        solar::rocket::engineToJson(original.stages[0].engine), errors);
    REQUIRE(engine);
    CHECK(errors.empty());
    CHECK(engine->thrustVacuumN == original.stages[0].engine.thrustVacuumN);
}

TEST_CASE("Invalid JSON values are rejected without a vehicle", "[rocket][json][validation]")
{
    auto json = solar::rocket::vehicleToJson(solar::test::twoStageVehicle());
    json["payloadMassKg"] = -1.0;
    const auto loaded = solar::rocket::vehicleFromJson(json);
    CHECK_FALSE(loaded.valid());
    CHECK_FALSE(loaded.vehicle);
    CHECK_FALSE(loaded.errors.empty());
}

TEST_CASE("Engine database schema loads validated convenience entries", "[rocket][json][engine]")
{
    const nlohmann::json json{
        {"schemaVersion", 1},
        {"engines", nlohmann::json::array({solar::rocket::engineToJson(
            solar::test::testEngine())})},
    };
    const auto loaded = solar::rocket::parseEngineDatabase(json);
    REQUIRE(loaded.valid());
    REQUIRE(loaded.database.engines().size() == 1);
    REQUIRE(loaded.database.find("analytical test engine"));
    CHECK(loaded.database.find("analytical test engine")->ispVacuumSec == 320.0);
}

TEST_CASE("Single two and three-stage engineering scenarios remain finite", "[rocket][analysis][scenarios]")
{
    auto vehicle = solar::test::twoStageVehicle();
    vehicle.stages.resize(1);
    const auto single = solar::rocket::analyzeVehicle(
        vehicle, solar::rocket::PerformanceEnvironment::vacuum, 9.80665);
    REQUIRE(single.valid());
    REQUIRE(single.stages.size() == 1);
    CHECK(single.totalIdealDeltaVMetersPerSec > 0.0);

    vehicle = solar::test::twoStageVehicle();
    const auto two = solar::rocket::analyzeVehicle(
        vehicle, solar::rocket::PerformanceEnvironment::vacuum, 9.80665);
    REQUIRE(two.valid());
    REQUIRE(two.stages.size() == 2);

    vehicle.stages.push_back(solar::test::testStage("Stage 3", 1'000.0, 4'000.0));
    const auto three = solar::rocket::analyzeVehicle(
        vehicle, solar::rocket::PerformanceEnvironment::vacuum, 9.80665);
    REQUIRE(three.valid());
    REQUIRE(three.stages.size() == 3);
    CHECK(three.stages[2].postSeparationMassKg == vehicle.payloadMassKg);
    CHECK(three.totalIdealDeltaVMetersPerSec > 0.0);
}
