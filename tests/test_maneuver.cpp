#include "astrodynamics/Maneuver.hpp"
#include "astrodynamics/ManeuverFrame.hpp"
#include "astrodynamics/OrbitConversions.hpp"
#include "astrodynamics/OrbitalMechanics.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/geometric.hpp>

#include <cmath>

namespace {
constexpr double kEarthMu = 398600.4418;
constexpr double kRadius = 6578.137;

solar::astronomy::StateVector circularState()
{
    return {{kRadius, 0.0, 0.0},
            {0.0, *solar::astrodynamics::circularVelocityKmPerSec(kEarthMu, kRadius), 0.0}};
}
}

TEST_CASE("Circular-orbit maneuver frame is orthonormal", "[maneuver]")
{
    const auto frame = solar::astrodynamics::makeManeuverFrame(circularState());
    REQUIRE(frame);
    CHECK(glm::length(frame->tangential) == Catch::Approx(1.0));
    CHECK(glm::length(frame->normal) == Catch::Approx(1.0));
    CHECK(glm::length(frame->radialOut) == Catch::Approx(1.0));
    CHECK(glm::dot(frame->tangential, frame->normal) == Catch::Approx(0.0).margin(1.0e-12));
    CHECK(glm::dot(frame->tangential, frame->radialOut) == Catch::Approx(0.0).margin(1.0e-12));
    CHECK(glm::dot(frame->normal, frame->radialOut) == Catch::Approx(0.0).margin(1.0e-12));
}

TEST_CASE("RTN maneuver frame remains orthonormal with radial velocity", "[maneuver][regression]")
{
    const solar::astronomy::StateVector eccentricState{{7000.0, 0.0, 0.0}, {1.0, 7.5, 0.0}};
    const auto frame = solar::astrodynamics::makeManeuverFrame(eccentricState);
    REQUIRE(frame);
    CHECK(glm::length(frame->radialOut) == Catch::Approx(1.0).margin(1.0e-14));
    CHECK(glm::length(frame->tangential) == Catch::Approx(1.0).margin(1.0e-14));
    CHECK(glm::length(frame->normal) == Catch::Approx(1.0).margin(1.0e-14));
    CHECK(glm::dot(frame->radialOut, frame->tangential) == Catch::Approx(0.0).margin(1.0e-14));
    CHECK(glm::dot(frame->radialOut, frame->normal) == Catch::Approx(0.0).margin(1.0e-14));
    CHECK(glm::dot(frame->tangential, frame->normal) == Catch::Approx(0.0).margin(1.0e-14));
}

TEST_CASE("RTN component norm equals generated delta-v norm", "[maneuver][regression]")
{
    const solar::astronomy::StateVector eccentricState{{7000.0, 0.0, 0.0}, {1.0, 7.5, 0.0}};
    const solar::astrodynamics::ImpulsiveManeuver maneuver{0.1, 0.0, 0.1};
    const auto deltaVelocity = solar::astrodynamics::maneuverDeltaVelocity(
        eccentricState, maneuver);
    REQUIRE(deltaVelocity);
    CHECK(glm::length(*deltaVelocity) == Catch::Approx(std::sqrt(0.02)).margin(1.0e-14));
    CHECK(glm::length(*deltaVelocity) == Catch::Approx(maneuver.magnitudeKmPerSec()).margin(1.0e-14));
}

TEST_CASE("Single-axis maneuvers align with RTN basis", "[maneuver][regression]")
{
    const solar::astronomy::StateVector eccentricState{{7000.0, 0.0, 0.0}, {1.0, 7.5, 0.2}};
    const auto frame = solar::astrodynamics::makeManeuverFrame(eccentricState);
    REQUIRE(frame);
    const auto tangential = solar::astrodynamics::maneuverDeltaVelocity(
        eccentricState, {0.1, 0.0, 0.0});
    const auto normal = solar::astrodynamics::maneuverDeltaVelocity(
        eccentricState, {0.0, 0.1, 0.0});
    const auto radial = solar::astrodynamics::maneuverDeltaVelocity(
        eccentricState, {0.0, 0.0, 0.1});
    REQUIRE(tangential);
    REQUIRE(normal);
    REQUIRE(radial);
    CHECK(glm::length(*tangential / 0.1 - frame->tangential) == Catch::Approx(0.0).margin(1.0e-14));
    CHECK(glm::length(*normal / 0.1 - frame->normal) == Catch::Approx(0.0).margin(1.0e-14));
    CHECK(glm::length(*radial / 0.1 - frame->radialOut) == Catch::Approx(0.0).margin(1.0e-14));
}

TEST_CASE("A prograde burn raises apoapsis and preserves burn-point periapsis", "[maneuver]")
{
    const auto result = solar::astrodynamics::applyManeuver(
        circularState(), {0.1, 0.0, 0.0});
    REQUIRE(result);
    const auto elements = solar::astrodynamics::stateToOrbitalElements(*result, kEarthMu);
    REQUIRE(elements);
    REQUIRE(elements->apoapsisRadiusKm());
    CHECK(*elements->apoapsisRadiusKm() > kRadius);
    CHECK(elements->periapsisRadiusKm() == Catch::Approx(kRadius).margin(1.0e-6));
}

TEST_CASE("A retrograde burn lowers the opposite periapsis", "[maneuver]")
{
    const auto result = solar::astrodynamics::applyManeuver(
        circularState(), {-0.1, 0.0, 0.0});
    REQUIRE(result);
    const auto elements = solar::astrodynamics::stateToOrbitalElements(*result, kEarthMu);
    REQUIRE(elements);
    REQUIRE(elements->apoapsisRadiusKm());
    CHECK(elements->periapsisRadiusKm() < kRadius);
    CHECK(*elements->apoapsisRadiusKm() == Catch::Approx(kRadius).margin(1.0e-6));
}

TEST_CASE("Normal and radial burns alter physically meaningful elements", "[maneuver]")
{
    const auto normalState = solar::astrodynamics::applyManeuver(
        circularState(), {0.0, 0.2, 0.0});
    const auto radialState = solar::astrodynamics::applyManeuver(
        circularState(), {0.0, 0.0, 0.2});
    REQUIRE(normalState);
    REQUIRE(radialState);
    const auto normalElements = solar::astrodynamics::stateToOrbitalElements(*normalState, kEarthMu);
    const auto radialElements = solar::astrodynamics::stateToOrbitalElements(*radialState, kEarthMu);
    REQUIRE(normalElements);
    REQUIRE(radialElements);
    CHECK(normalElements->inclinationRad > 0.0);
    CHECK(radialElements->eccentricity > 0.0);
}

TEST_CASE("Circularization and opposite-apsis helpers use the shared equations", "[maneuver]")
{
    const auto raised = solar::astrodynamics::applyManeuver(circularState(), {0.1, 0.0, 0.0});
    REQUIRE(raised);
    REQUIRE(solar::astrodynamics::circularizationDeltaVelocityKmPerSec(*raised, kEarthMu));
    CHECK(*solar::astrodynamics::circularizationDeltaVelocityKmPerSec(*raised, kEarthMu) ==
          Catch::Approx(-0.1).margin(1.0e-10));
    const auto targetBurn = solar::astrodynamics::apsidalTargetDeltaVelocityKmPerSec(
        circularState(), kRadius + 1000.0, kEarthMu);
    REQUIRE(targetBurn);
    CHECK(*targetBurn > 0.0);
}
