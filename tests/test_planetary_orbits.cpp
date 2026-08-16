#include "TestKernels.hpp"
#include "astronomy/BodyDatabase.hpp"
#include "astronomy/Ephemeris.hpp"
#include "astronomy/Orientation.hpp"
#include "astronomy/SpiceContext.hpp"
#include "astrodynamics/Angles.hpp"
#include "astrodynamics/OrbitConversions.hpp"
#include "astrodynamics/OsculatingOrbit.hpp"
#include "core/Time.hpp"
#include "scene/SolarSystemScene.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <numbers>
#include <string_view>

namespace {

[[nodiscard]] const solar::scene::SceneObject* findObject(
    const solar::scene::SolarSystemScene& scene, const std::string_view name)
{
    const auto found = std::find_if(
        scene.objects().begin(), scene.objects().end(), [&](const auto& object) {
            return object.body->name() == name;
        });
    return found == scene.objects().end() ? nullptr : &*found;
}

void checkFiniteBoundOrbit(const solar::scene::SceneObject& object)
{
    REQUIRE(object.osculatingOrbit);
    const auto& elements = object.osculatingOrbit->elements;
    CHECK(std::isfinite(elements.semiMajorAxisKm));
    CHECK(elements.semiMajorAxisKm > 0.0);
    CHECK(std::isfinite(elements.eccentricity));
    CHECK(elements.eccentricity >= 0.0);
    CHECK(elements.eccentricity < 1.0);
    CHECK(std::isfinite(elements.inclinationRad));
    CHECK(std::isfinite(elements.longitudeAscendingNodeRad));
    CHECK(std::isfinite(elements.argumentPeriapsisRad));
    CHECK(std::isfinite(elements.trueAnomalyRad));
}

} // namespace

TEST_CASE("SPICE planetary states produce current primary-relative osculating elements",
          "[astronomy][spice][osculating-orbit]")
{
    solar::astronomy::SpiceContext spice;
    REQUIRE(spice.loadKernelManifest(solar::test::kernelDirectory()));
    const solar::astronomy::BodyDatabase database;
    solar::astronomy::Ephemeris ephemeris(spice);
    solar::astronomy::Orientation orientation(spice);
    solar::scene::SolarSystemScene scene(database, ephemeris, orientation, spice);
    const auto epoch = solar::core::makeUtcTimePoint({2031, 4, 21, 13, 42, 18});
    REQUIRE(epoch);
    REQUIRE(scene.update(*epoch));
    CHECK(scene.referenceFrame().axes == solar::astronomy::AxisFrame::eclipticJ2000);

    constexpr std::array planetNames{
        "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"};
    for (const std::string_view name : planetNames) {
        CAPTURE(name);
        const auto* object = findObject(scene, name);
        REQUIRE(object != nullptr);
        REQUIRE(object->orbitCentralBody != nullptr);
        CHECK(object->orbitCentralBody->name() == "Sun");
        checkFiniteBoundOrbit(*object);
    }

    const auto* mercury = findObject(scene, "Mercury");
    const auto* mars = findObject(scene, "Mars");
    REQUIRE(mercury != nullptr);
    REQUIRE(mars != nullptr);
    CHECK(mercury->osculatingOrbit->elements.eccentricity > 0.0);
    CHECK(mercury->osculatingOrbit->elements.inclinationRad > 0.0);
    CHECK(mars->osculatingOrbit->elements.inclinationRad > 0.0);

    const auto* moon = findObject(scene, "Moon");
    REQUIRE(moon != nullptr);
    REQUIRE(moon->orbitCentralBody != nullptr);
    CHECK(moon->orbitCentralBody->name() == "Earth");
    checkFiniteBoundOrbit(*moon);

    const auto* uranus = findObject(scene, "Uranus");
    REQUIRE(uranus != nullptr);
    REQUIRE(uranus->osculatingOrbit);
    const glm::dvec3 rotationAxis = glm::normalize(
        uranus->bodyFixedToReference * glm::dvec3{0.0, 0.0, 1.0});
    CHECK(std::abs(glm::dot(rotationAxis, uranus->osculatingOrbit->orbitalNormal)) < 0.5);
}

TEST_CASE("Osculating plane basis follows r cross v and nodes lie on the reference plane",
          "[astrodynamics][osculating-orbit]")
{
    constexpr double earthMu = 398'600.435436;
    const solar::astrodynamics::OrbitalElements elements{
        12'000.0, 0.24, 0.71, 1.03, 0.47, 2.1, 12'000.0 * (1.0 - 0.24 * 0.24)};
    const auto state = solar::astrodynamics::orbitalElementsToState(elements, earthMu);
    REQUIRE(state);
    const auto orbit = solar::astrodynamics::deriveOsculatingOrbit(*state, earthMu);
    REQUIRE(orbit);

    const glm::dvec3 expectedH = glm::cross(state->positionKm, state->velocityKmPerSec);
    CHECK(glm::length(orbit->angularMomentumKm2PerSec - expectedH) ==
          Catch::Approx(0.0).margin(1.0e-10));
    CHECK(glm::length(orbit->orbitalNormal) == Catch::Approx(1.0).margin(1.0e-12));
    CHECK(glm::dot(orbit->planeBasisU, orbit->orbitalNormal) ==
          Catch::Approx(0.0).margin(1.0e-12));
    CHECK(glm::dot(orbit->planeBasisV, orbit->orbitalNormal) ==
          Catch::Approx(0.0).margin(1.0e-12));
    CHECK(glm::dot(orbit->planeBasisU, orbit->planeBasisV) ==
          Catch::Approx(0.0).margin(1.0e-12));
    REQUIRE(orbit->ascendingNodePositionKm);
    REQUIRE(orbit->descendingNodePositionKm);
    CHECK(orbit->ascendingNodePositionKm->z == Catch::Approx(0.0).margin(1.0e-8));
    CHECK(orbit->descendingNodePositionKm->z == Catch::Approx(0.0).margin(1.0e-8));

    const double independentInclination = std::acos(
        expectedH.z / glm::length(expectedH));
    CHECK(orbit->elements.inclinationRad ==
          Catch::Approx(independentInclination).margin(1.0e-12));
}

TEST_CASE("Osculating geometry rejects invalid central-body and degenerate states",
          "[astrodynamics][osculating-orbit][error]")
{
    const solar::astronomy::StateVector validState{{7'000.0, 0.0, 0.0},
                                                    {0.0, 7.5, 1.0}};
    CHECK_FALSE(solar::astrodynamics::deriveOsculatingOrbit(validState, 0.0));
    CHECK_FALSE(solar::astrodynamics::deriveOsculatingOrbit(
        solar::astronomy::StateVector{}, 398'600.435436));
}

TEST_CASE("Mercury SPICE orbit samples retain out-of-ecliptic geometry",
          "[scene][spice][orbit-3d][regression]")
{
    solar::astronomy::SpiceContext spice;
    REQUIRE(spice.loadKernelManifest(solar::test::kernelDirectory()));
    const solar::astronomy::BodyDatabase database;
    solar::astronomy::Ephemeris ephemeris(spice);
    solar::astronomy::Orientation orientation(spice);
    solar::scene::SolarSystemScene scene(database, ephemeris, orientation, spice);
    const auto epoch = solar::core::makeUtcTimePoint({2031, 4, 21, 13, 42, 18});
    REQUIRE(epoch);
    REQUIRE(scene.update(*epoch));
    const auto* mercury = findObject(scene, "Mercury");
    REQUIRE(mercury != nullptr);
    REQUIRE(mercury->orbitReferencePointsKm.size() > 100);
    REQUIRE(mercury->orbitRenderPoints.size() == mercury->orbitReferencePointsKm.size());

    double minimumScientificZ = std::numeric_limits<double>::infinity();
    double maximumScientificZ = -std::numeric_limits<double>::infinity();
    float minimumRenderY = std::numeric_limits<float>::infinity();
    float maximumRenderY = -std::numeric_limits<float>::infinity();
    for (std::size_t index = 0; index < mercury->orbitReferencePointsKm.size(); ++index) {
        minimumScientificZ = std::min(
            minimumScientificZ, mercury->orbitReferencePointsKm[index].z);
        maximumScientificZ = std::max(
            maximumScientificZ, mercury->orbitReferencePointsKm[index].z);
        minimumRenderY = std::min(minimumRenderY, mercury->orbitRenderPoints[index].y);
        maximumRenderY = std::max(maximumRenderY, mercury->orbitRenderPoints[index].y);
    }
    CHECK(maximumScientificZ - minimumScientificZ > 1'000'000.0);
    CHECK(maximumRenderY - minimumRenderY > 0.01F);

    const auto* mars = findObject(scene, "Mars");
    REQUIRE(mars != nullptr);
    REQUIRE(mars->orbitReferencePointsKm.size() > 100);
    double maximumMarsRelativeHeight = 0.0;
    for (const auto& point : mars->orbitReferencePointsKm) {
        maximumMarsRelativeHeight = std::max(
            maximumMarsRelativeHeight, std::abs(point.z) / glm::length(point));
    }
    CHECK(maximumMarsRelativeHeight > 0.01);
}

TEST_CASE("Nearby SPICE epochs keep osculating elements finite while anomaly evolves",
          "[scene][spice][osculating-orbit][time]")
{
    solar::astronomy::SpiceContext spice;
    REQUIRE(spice.loadKernelManifest(solar::test::kernelDirectory()));
    const solar::astronomy::BodyDatabase database;
    solar::astronomy::Ephemeris ephemeris(spice);
    solar::astronomy::Orientation orientation(spice);
    solar::scene::SolarSystemScene scene(database, ephemeris, orientation, spice);
    const auto epoch = solar::core::makeUtcTimePoint({2031, 4, 21, 13, 42, 18});
    REQUIRE(epoch);
    REQUIRE(scene.update(*epoch));
    const auto* mars = findObject(scene, "Mars");
    REQUIRE(mars != nullptr);
    REQUIRE(mars->osculatingOrbit);
    const auto anomalyAtEpoch = mars->osculatingOrbit->elements.trueAnomalyRad;

    REQUIRE(scene.update(*epoch + std::chrono::hours{24}));
    mars = findObject(scene, "Mars");
    REQUIRE(mars != nullptr);
    checkFiniteBoundOrbit(*mars);
    CHECK(std::abs(solar::astrodynamics::angularDifference(
              mars->osculatingOrbit->elements.trueAnomalyRad, anomalyAtEpoch)) > 1.0e-5);
}
