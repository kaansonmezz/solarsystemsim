#include "RocketFixtures.hpp"

#include "rocket/Engine.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Engine selects vacuum and sea-level performance", "[rocket][engine]")
{
    const auto engine = solar::test::testEngine();
    CHECK(engine.thrustN(solar::rocket::PerformanceEnvironment::vacuum) == 1'000'000.0);
    CHECK(engine.thrustN(solar::rocket::PerformanceEnvironment::seaLevel) == 900'000.0);
    CHECK(engine.ispSec(solar::rocket::PerformanceEnvironment::vacuum) == 320.0);
    CHECK(engine.ispSec(solar::rocket::PerformanceEnvironment::seaLevel) == 300.0);
    CHECK(solar::rocket::validateEngine(engine).valid());
}

TEST_CASE("Invalid engine definitions report errors", "[rocket][engine][validation]")
{
    auto engine = solar::test::testEngine();
    engine.thrustVacuumN = -1.0;
    engine.ispSeaLevelSec = 0.0;
    engine.massKg = -2.0;
    engine.minimumThrottle = 0.9;
    engine.maximumThrottle = 0.2;
    CHECK_FALSE(solar::rocket::validateEngine(engine).valid());
    CHECK(solar::rocket::validateEngine(engine).errors.size() == 4);
}
