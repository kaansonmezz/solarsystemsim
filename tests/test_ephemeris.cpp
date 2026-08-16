#include "astronomy/BodyDatabase.hpp"
#include "astronomy/Ephemeris.hpp"
#include "astronomy/Orientation.hpp"
#include "astronomy/SpiceContext.hpp"
#include "scene/SolarSystemScene.hpp"
#include "simulation/SimulationClock.hpp"
#include "core/Time.hpp"
#include "TestKernels.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>

TEST_CASE("SimulationClock time maps directly to SPICE epochs including reverse", "[astronomy][time]")
{
    solar::astronomy::SpiceContext spice;
    REQUIRE(spice.loadKernelManifest(solar::test::kernelDirectory()));
    solar::astronomy::Ephemeris ephemeris(spice);
    const solar::astronomy::BodyDatabase database;
    const auto* earth = database.find("Earth");
    REQUIRE(earth != nullptr);

    const auto initial = std::chrono::system_clock::time_point{std::chrono::seconds{1'893'456'000}};
    solar::simulation::SimulationClock clock(initial);
    clock.setTimeScale(100'000.0);
    const auto et0 = ephemeris.ephemerisTime(clock.simulationTime());
    REQUIRE(et0.has_value());
    clock.advance(1.0);
    const auto etForward = ephemeris.ephemerisTime(clock.simulationTime());
    REQUIRE(etForward.has_value());
    CHECK(*etForward - *et0 == Catch::Approx(100'000.0).margin(1.0e-3));

    clock.setTimeScale(-100'000.0);
    clock.advance(2.0);
    const auto etReverse = ephemeris.ephemerisTime(clock.simulationTime());
    REQUIRE(etReverse.has_value());
    CHECK(*etReverse - *etForward == Catch::Approx(-200'000.0).margin(1.0e-3));

    const auto state = ephemeris.getState(*earth, 10, solar::astronomy::AxisFrame::j2000,
                                          clock.simulationTime());
    REQUIRE(state.has_value());
    CHECK(solar::astronomy::isFinite(*state));
}

TEST_CASE("Chrono UTC maps to SPICE ET across supported historical and future dates",
          "[astronomy][time]")
{
    solar::astronomy::SpiceContext spice;
    REQUIRE(spice.loadKernelManifest(solar::test::kernelDirectory()));
    solar::astronomy::Ephemeris ephemeris(spice);
    double previousEt = -1.0e100;
    for (const int year : {1900, 2000, 2031, 2100}) {
        const auto utc = solar::core::makeUtcTimePoint({year, 4, 21, 13, 42, 18});
        REQUIRE(utc);
        const auto et = ephemeris.ephemerisTime(*utc);
        REQUIRE(et);
        CHECK(*et > previousEt);
        previousEt = *et;
    }
}

TEST_CASE("SolarSystemScene evaluates positions and orientation directly from time", "[scene][astronomy]")
{
    solar::astronomy::SpiceContext spice;
    REQUIRE(spice.loadKernelManifest(solar::test::kernelDirectory()));
    const solar::astronomy::BodyDatabase database;
    solar::astronomy::Ephemeris ephemeris(spice);
    solar::astronomy::Orientation orientation(spice);
    solar::scene::SolarSystemScene scene(database, ephemeris, orientation, spice);

    const auto epoch = std::chrono::system_clock::time_point{std::chrono::seconds{1'893'456'000}};
    REQUIRE(scene.update(epoch));
    REQUIRE(scene.selectBody(499));
    const auto marsAtEpoch = scene.selectedObject()->barycentricState.positionKm;
    const auto orientationAtEpoch = scene.selectedObject()->bodyFixedToReference;

    REQUIRE(scene.update(epoch + std::chrono::hours{24 * 100}));
    const auto marsLater = scene.selectedObject()->barycentricState.positionKm;
    const auto orientationLater = scene.selectedObject()->bodyFixedToReference;
    CHECK(glm::distance(marsAtEpoch, marsLater) > 1'000'000.0);
    CHECK(orientationAtEpoch != orientationLater);

    REQUIRE(scene.update(epoch));
    CHECK(glm::distance(marsAtEpoch, scene.selectedObject()->barycentricState.positionKm) < 1.0e-6);
    CHECK(scene.selectedObject()->bodyFixedToReference == orientationAtEpoch);

    scene.selectBody(399);
    scene.setReferenceOrigin(solar::astronomy::OriginMode::earth);
    REQUIRE(scene.update(epoch));
    REQUIRE(scene.selectedObject() != nullptr);
    CHECK(glm::length(scene.selectedObject()->referenceState.positionKm) < 1.0e-9);
}

TEST_CASE("Large simulation-time jumps invalidate planetary orbit caches", "[scene][cache]")
{
    solar::astronomy::SpiceContext spice;
    REQUIRE(spice.loadKernelManifest(solar::test::kernelDirectory()));
    const solar::astronomy::BodyDatabase database;
    solar::astronomy::Ephemeris ephemeris(spice);
    solar::astronomy::Orientation orientation(spice);
    solar::scene::SolarSystemScene scene(database, ephemeris, orientation, spice);
    const auto epoch = std::chrono::system_clock::time_point{std::chrono::seconds{1'893'456'000}};
    REQUIRE(scene.update(epoch));
    const auto initialRevision = scene.orbitRevision();
    const auto* earth = database.find("Earth");
    REQUIRE(earth != nullptr);
    const auto* earthObject = [&]() -> const solar::scene::SceneObject* {
        for (const auto& object : scene.objects()) {
            if (object.body == earth) return &object;
        }
        return nullptr;
    }();
    REQUIRE(earthObject != nullptr);
    REQUIRE(earthObject->orbitCacheEpoch);
    CHECK_FALSE(scene.orbitCacheIsStale(*earthObject, epoch + std::chrono::hours{24 * 10}));
    CHECK(scene.orbitCacheIsStale(*earthObject, epoch + std::chrono::hours{24 * 365}));
    REQUIRE(scene.update(epoch + std::chrono::hours{24 * 365}));
    CHECK(scene.orbitRevision() > initialRevision);
    REQUIRE(earthObject->orbitCacheEpoch);
    CHECK(*earthObject->orbitCacheEpoch == epoch + std::chrono::hours{24 * 365});
}
