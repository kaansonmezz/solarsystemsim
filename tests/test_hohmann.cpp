#include "astrodynamics/Hohmann.hpp"
#include "astrodynamics/OrbitalMechanics.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("LEO Hohmann transfer matches an independent reference", "[hohmann]")
{
    // Earth radii 6578.137 -> 7378.137 km, independently tabulated to millimetres/s scale.
    const auto transfer = solar::astrodynamics::calculateHohmannTransfer(
        6578.137, 7378.137, 398600.4418);
    REQUIRE(transfer);
    CHECK(transfer->firstBurnKmPerSec == Catch::Approx(0.219996).margin(2.0e-6));
    CHECK(transfer->secondBurnKmPerSec == Catch::Approx(0.213771).margin(2.0e-6));
    CHECK(transfer->totalDeltaVelocityKmPerSec == Catch::Approx(0.433766).margin(3.0e-6));
    CHECK(transfer->transferTimeSeconds == Catch::Approx(2900.6).margin(2.0));
}

TEST_CASE("Spacecraft Hohmann helper derives its initial radius from current state",
          "[hohmann][regression]")
{
    constexpr double mu = 398600.4418;
    constexpr double actualRadius = 6578.137;
    constexpr double targetRadius = 7378.137;
    const solar::astronomy::StateVector state{
        {actualRadius, 0.0, 0.0},
        {0.0, *solar::astrodynamics::circularVelocityKmPerSec(mu, actualRadius), 0.0}};
    const auto current = solar::astrodynamics::calculateHohmannFromCurrentCircularState(
        state, targetRadius, mu);
    REQUIRE(current);
    CHECK(current->initialRadiusKm == Catch::Approx(actualRadius));
    CHECK(current->targetRadiusKm == Catch::Approx(targetRadius));
    const auto direct = solar::astrodynamics::calculateHohmannTransfer(
        actualRadius, targetRadius, mu);
    REQUIRE(direct);
    CHECK(current->transfer.firstBurnKmPerSec ==
          Catch::Approx(direct->firstBurnKmPerSec).margin(1.0e-14));
}

TEST_CASE("Spacecraft Hohmann helper rejects an eccentric current orbit",
          "[hohmann][regression]")
{
    const solar::astronomy::StateVector eccentric{{7000.0, 0.0, 0.0}, {1.0, 7.5, 0.0}};
    CHECK_FALSE(solar::astrodynamics::calculateHohmannFromCurrentCircularState(
        eccentric, 9000.0, 398600.4418));
}

TEST_CASE("Inward Hohmann burns have consistent negative signs", "[hohmann]")
{
    const auto transfer = solar::astrodynamics::calculateHohmannTransfer(
        7378.137, 6578.137, 398600.4418);
    REQUIRE(transfer);
    CHECK(transfer->firstBurnKmPerSec < 0.0);
    CHECK(transfer->secondBurnKmPerSec < 0.0);
    CHECK(transfer->totalDeltaVelocityKmPerSec > 0.0);
}
