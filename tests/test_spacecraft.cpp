#include "simulation/Spacecraft.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/geometric.hpp>

#include <chrono>

namespace {
constexpr double kEarthMu = 398600.4418;
constexpr double kEarthRadius = 6378.137;
}

TEST_CASE("Spacecraft state is evaluated from authoritative epoch in either time direction",
          "[spacecraft]")
{
    const solar::core::SimulationTimePoint epoch{std::chrono::seconds{1'000'000}};
    auto spacecraft = solar::simulation::Spacecraft::fromApsisAltitudes(
        "Test", "Earth", 399, kEarthRadius, kEarthMu,
        200.0, 200.0, 0.4, 0.2, 0.1, 0.0, epoch);
    REQUIRE(spacecraft);
    const auto initial = spacecraft->current().relativeState;

    spacecraft->update(epoch + std::chrono::seconds{900});
    CHECK(glm::length(spacecraft->current().relativeState.positionKm - initial.positionKm) > 1.0);
    spacecraft->update(epoch);
    CHECK(glm::length(spacecraft->current().relativeState.positionKm - initial.positionKm) ==
          Catch::Approx(0.0).margin(1.0e-7));
    spacecraft->update(epoch - std::chrono::seconds{900});
    CHECK(solar::astronomy::isFinite(spacecraft->current().relativeState));
}

TEST_CASE("Spacecraft maneuver preview is non-destructive until applied", "[spacecraft]")
{
    const solar::core::SimulationTimePoint epoch{};
    auto spacecraft = solar::simulation::Spacecraft::fromApsisAltitudes(
        "Test", "Earth", 399, kEarthRadius, kEarthMu,
        200.0, 200.0, 0.0, 0.0, 0.0, 0.0, epoch);
    REQUIRE(spacecraft);
    const auto original = spacecraft->orbit().stateAtEpoch();
    REQUIRE(spacecraft->previewManeuver({0.1, 0.0, 0.0}, epoch));
    REQUIRE(spacecraft->previewOrbit());
    CHECK(glm::length(spacecraft->orbit().stateAtEpoch().velocityKmPerSec -
                      original.velocityKmPerSec) == Catch::Approx(0.0));
    REQUIRE(spacecraft->applyPreview());
    CHECK(glm::length(spacecraft->orbit().stateAtEpoch().velocityKmPerSec -
                      original.velocityKmPerSec) == Catch::Approx(0.1).margin(1.0e-12));
}

TEST_CASE("Spacecraft produces renderer-independent scientific orbit geometry",
          "[spacecraft][visualization]")
{
    const solar::core::SimulationTimePoint epoch{};
    auto spacecraft = solar::simulation::Spacecraft::fromApsisAltitudes(
        "Test", "Earth", 399, kEarthRadius, kEarthMu,
        200.0, 1000.0, 0.4, 0.2, 0.1, 0.0, epoch);
    REQUIRE(spacecraft);
    const auto initialGeometryRevision = spacecraft->visualizationData().geometryRevision;
    REQUIRE(spacecraft->visualizationData().trajectoryPointsKm.size() == 256);
    CHECK(spacecraft->visualizationData().markers.size() == 4);
    REQUIRE(spacecraft->visualizationData().radialDirection);
    REQUIRE(spacecraft->visualizationData().tangentialDirection);
    REQUIRE(spacecraft->visualizationData().normalDirection);

    spacecraft->update(epoch + std::chrono::seconds{30});
    CHECK(spacecraft->visualizationData().geometryRevision == initialGeometryRevision);
    CHECK(spacecraft->visualizationData().stateRevision > 0);
    REQUIRE(spacecraft->previewManeuver({0.1, 0.0, 0.0}, epoch + std::chrono::seconds{30}));
    CHECK(spacecraft->visualizationData().geometryRevision > initialGeometryRevision);
    CHECK_FALSE(spacecraft->visualizationData().previewTrajectoryPointsKm.empty());
    REQUIRE(spacecraft->visualizationData().maneuverPositionKm);
    REQUIRE(spacecraft->visualizationData().deltaVelocityDirection);
}
