#include "RocketFixtures.hpp"

#include "rocket/MassFlow.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Mass flow follows thrust over Isp g0", "[rocket][mass-flow]")
{
    const auto flow = solar::rocket::massFlowKgPerSec(900'000.0, 300.0);
    REQUIRE(flow);
    CHECK(*flow == Catch::Approx(900'000.0 / (300.0 * 9.80665)).epsilon(1.0e-13));
}

TEST_CASE("Engine cluster scales thrust and mass flow", "[rocket][mass-flow][cluster]")
{
    const auto engine = solar::test::testEngine();
    const auto thrust = solar::rocket::engineClusterThrustN(
        engine, 9, solar::rocket::PerformanceEnvironment::vacuum);
    const auto flow = solar::rocket::engineClusterMassFlowKgPerSec(
        engine, 9, solar::rocket::PerformanceEnvironment::vacuum);
    REQUIRE(thrust);
    REQUIRE(flow);
    CHECK(*thrust == 9'000'000.0);
    CHECK(*flow == Catch::Approx(9.0 * 1'000'000.0 / (320.0 * 9.80665)));
    CHECK_FALSE(solar::rocket::engineClusterThrustN(
        engine, 0, solar::rocket::PerformanceEnvironment::vacuum));
}
