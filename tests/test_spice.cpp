#include "astronomy/SpiceContext.hpp"
#include "TestKernels.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <stdexcept>

TEST_CASE("SPICE kernels load and provide controlled queries", "[astronomy][spice]")
{
    solar::astronomy::SpiceContext spice;
    REQUIRE(spice.loadKernelManifest(solar::test::kernelDirectory()));
    CHECK(spice.isReady());
    CHECK(spice.loadedKernels().size() == 3);

    const auto et = spice.utcToEphemerisTime("2031-04-21T13:42:18 UTC");
    REQUIRE(et.has_value());
    CHECK(std::isfinite(*et));

    const auto earth = spice.getState(399, 10, "J2000", *et);
    REQUIRE(earth.has_value());
    CHECK(solar::astronomy::isFinite(*earth));
    CHECK(glm::length(earth->positionKm) > 100'000'000.0);

    const auto earthToEarth = spice.getState(399, 399, "J2000", *et);
    REQUIRE(earthToEarth.has_value());
    CHECK(glm::length(earthToEarth->positionKm) < 1.0e-9);
    CHECK(glm::length(earthToEarth->velocityKmPerSec) < 1.0e-12);
}

TEST_CASE("SPICE invalid inputs fail without terminating", "[astronomy][spice]")
{
    solar::astronomy::SpiceContext spice;
    CHECK_FALSE(spice.loadKernel(solar::test::kernelDirectory() / "missing.bsp"));
    CHECK_FALSE(spice.lastError().empty());

    REQUIRE(spice.loadKernelManifest(solar::test::kernelDirectory()));
    const auto et = spice.utcToEphemerisTime("2030-01-01 UTC");
    REQUIRE(et.has_value());
    CHECK_FALSE(spice.getState(123456789, 10, "J2000", *et).has_value());
    CHECK_FALSE(spice.lastError().empty());
}

TEST_CASE("SPICE rejects epochs outside loaded ephemeris coverage", "[astronomy][spice]")
{
    solar::astronomy::SpiceContext spice;
    REQUIRE(spice.loadKernelManifest(solar::test::kernelDirectory()));
    const auto et = spice.utcToEphemerisTime("1800-01-01 UTC");
    REQUIRE(et.has_value());
    CHECK_FALSE(spice.getState(399, 10, "J2000", *et).has_value());
    CHECK_FALSE(spice.lastError().empty());
}

TEST_CASE("SPICE global kernel ownership permits only one active context", "[astronomy][spice]")
{
    solar::astronomy::SpiceContext owner;
    CHECK_THROWS_AS(solar::astronomy::SpiceContext{}, std::logic_error);
}
