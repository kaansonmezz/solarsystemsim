#include "astronomy/BodyDatabase.hpp"
#include "astronomy/Ephemeris.hpp"
#include "astronomy/ReferenceFrame.hpp"
#include "astronomy/SpiceContext.hpp"
#include "scene/RenderTransform.hpp"
#include "TestKernels.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/geometric.hpp>

TEST_CASE("Relative state matches barycentric subtraction", "[astronomy][frames]")
{
    solar::astronomy::SpiceContext spice;
    REQUIRE(spice.loadKernelManifest(solar::test::kernelDirectory()));
    solar::astronomy::Ephemeris ephemeris(spice);
    const solar::astronomy::BodyDatabase database;
    const auto* earth = database.find("Earth");
    const auto* sun = database.find("Sun");
    REQUIRE(earth != nullptr);
    REQUIRE(sun != nullptr);
    const auto et = spice.utcToEphemerisTime("2031-04-21T13:42:18 UTC");
    REQUIRE(et.has_value());
    const auto earthBarycentric = ephemeris.getStateAtEt(
        *earth, 0, solar::astronomy::AxisFrame::j2000, *et);
    const auto sunBarycentric = ephemeris.getStateAtEt(
        *sun, 0, solar::astronomy::AxisFrame::j2000, *et);
    const auto earthHeliocentric = ephemeris.getStateAtEt(
        *earth, 10, solar::astronomy::AxisFrame::j2000, *et);
    REQUIRE(earthBarycentric.has_value());
    REQUIRE(sunBarycentric.has_value());
    REQUIRE(earthHeliocentric.has_value());

    const auto subtracted = solar::astronomy::relativeState(*earthBarycentric, *sunBarycentric);
    CHECK(glm::distance(subtracted.positionKm, earthHeliocentric->positionKm) < 1.0e-5);
    CHECK(glm::distance(subtracted.velocityKmPerSec, earthHeliocentric->velocityKmPerSec) < 1.0e-10);
}

TEST_CASE("Render transform rebases and keeps scientific units separate", "[scene][frames]")
{
    solar::scene::RenderTransform transform;
    transform.setDistanceScale(10.0);
    const glm::dvec3 oneAuOnX{solar::scene::kAstronomicalUnitKm, 0.0, 0.0};
    const glm::vec3 rendered = transform.toRenderPosition(oneAuOnX);
    CHECK(rendered.x == Catch::Approx(10.0F));
    CHECK(rendered.y == Catch::Approx(0.0F));
    CHECK(rendered.z == Catch::Approx(0.0F));
    CHECK(oneAuOnX.x == solar::scene::kAstronomicalUnitKm);
}
