#include "app/AppConfig.hpp"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("Astronomy reference defaults parse from configuration", "[config]")
{
    const auto sun = solar::app::parseAppConfig(nlohmann::json{
        {"astronomy", {{"defaultOrigin", "Sun"}, {"defaultFrame", "ECLIPJ2000"}}}});
    CHECK(sun.config.defaultOrigin == solar::astronomy::OriginMode::sun);
    CHECK(sun.config.defaultFrame == solar::astronomy::AxisFrame::eclipticJ2000);
    CHECK(sun.warnings.empty());

    const auto earth = solar::app::parseAppConfig(nlohmann::json{
        {"astronomy", {{"defaultOrigin", "Earth"}, {"defaultFrame", "J2000"}}}});
    CHECK(earth.config.defaultOrigin == solar::astronomy::OriginMode::earth);
    CHECK(earth.config.defaultFrame == solar::astronomy::AxisFrame::j2000);
    CHECK(earth.warnings.empty());
}

TEST_CASE("Invalid astronomy reference defaults fall back safely", "[config]")
{
    const auto result = solar::app::parseAppConfig(nlohmann::json{
        {"astronomy", {{"defaultOrigin", "Mars-ish"}, {"defaultFrame", "RendererXY"}}}});
    CHECK(result.config.defaultOrigin == solar::astronomy::OriginMode::sun);
    CHECK(result.config.defaultFrame == solar::astronomy::AxisFrame::eclipticJ2000);
    CHECK(result.warnings.size() == 2);
}

TEST_CASE("Missing astronomy configuration uses documented defaults", "[config]")
{
    const auto result = solar::app::parseAppConfig(nlohmann::json::object());
    CHECK(result.config.defaultOrigin == solar::astronomy::OriginMode::sun);
    CHECK(result.config.defaultFrame == solar::astronomy::AxisFrame::eclipticJ2000);
    CHECK(result.config.kernelDirectory == "assets/kernels");
    CHECK(result.warnings.empty());
}

TEST_CASE("Camera orbit sensitivity parses and clamps", "[config][camera]")
{
    const auto configured = solar::app::parseAppConfig(nlohmann::json{
        {"renderer", {{"orbitSensitivity", 0.35}}}});
    CHECK(configured.config.orbitSensitivity == 0.35F);

    const auto tooLow = solar::app::parseAppConfig(nlohmann::json{
        {"renderer", {{"orbitSensitivity", -5.0}}}});
    CHECK(tooLow.config.orbitSensitivity == 0.01F);

    const auto tooHigh = solar::app::parseAppConfig(nlohmann::json{
        {"renderer", {{"orbitSensitivity", 20.0}}}});
    CHECK(tooHigh.config.orbitSensitivity == 2.0F);
}
