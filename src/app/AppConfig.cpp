#include "app/AppConfig.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <optional>
#include <string_view>

namespace solar::app {
namespace {

[[nodiscard]] std::optional<astronomy::OriginMode> originFromString(
    const std::string_view value) noexcept
{
    if (value == "Sun") return astronomy::OriginMode::sun;
    if (value == "SolarSystemBarycenter") return astronomy::OriginMode::solarSystemBarycenter;
    if (value == "Earth") return astronomy::OriginMode::earth;
    if (value == "SelectedBody") return astronomy::OriginMode::selectedBody;
    return std::nullopt;
}

[[nodiscard]] std::optional<astronomy::AxisFrame> frameFromString(
    const std::string_view value) noexcept
{
    if (value == "J2000") return astronomy::AxisFrame::j2000;
    if (value == "ECLIPJ2000") return astronomy::AxisFrame::eclipticJ2000;
    return std::nullopt;
}

} // namespace

AppConfigResult parseAppConfig(const nlohmann::json& json)
{
    AppConfigResult result;
    if (const auto window = json.find("window"); window != json.end() && window->is_object()) {
        result.config.windowWidth = window->value("width", result.config.windowWidth);
        result.config.windowHeight = window->value("height", result.config.windowHeight);
        result.config.verticalSync = window->value("vsync", result.config.verticalSync);
    }
    if (const auto renderer = json.find("renderer");
        renderer != json.end() && renderer->is_object()) {
        result.config.fieldOfView = renderer->value("fov", result.config.fieldOfView);
        result.config.orbitSensitivity = renderer->value(
            "orbitSensitivity", result.config.orbitSensitivity);
    }
    if (const auto astronomy = json.find("astronomy");
        astronomy != json.end() && astronomy->is_object()) {
        result.config.kernelDirectory = astronomy->value(
            "kernelDirectory", result.config.kernelDirectory);
        const std::string origin = astronomy->value("defaultOrigin", "Sun");
        if (const auto parsed = originFromString(origin)) {
            result.config.defaultOrigin = *parsed;
        } else {
            result.warnings.push_back("Invalid astronomy.defaultOrigin '" + origin +
                                      "'; using Sun");
        }
        const std::string frame = astronomy->value("defaultFrame", "ECLIPJ2000");
        if (const auto parsed = frameFromString(frame)) {
            result.config.defaultFrame = *parsed;
        } else {
            result.warnings.push_back("Invalid astronomy.defaultFrame '" + frame +
                                      "'; using ECLIPJ2000");
        }
    }
    if (const auto visualization = json.find("visualization");
        visualization != json.end() && visualization->is_object()) {
        result.config.distanceScale = visualization->value(
            "distanceScale", result.config.distanceScale);
        result.config.bodyRadiusScale = visualization->value(
            "bodyRadiusScale", result.config.bodyRadiusScale);
        result.config.showOrbits = visualization->value("showOrbits", result.config.showOrbits);
        result.config.showLabels = visualization->value("showLabels", result.config.showLabels);
    }
    result.config.windowWidth = std::clamp(result.config.windowWidth, 640, 7680);
    result.config.windowHeight = std::clamp(result.config.windowHeight, 480, 4320);
    result.config.fieldOfView = std::clamp(result.config.fieldOfView, 20.0F, 120.0F);
    result.config.orbitSensitivity = std::clamp(
        result.config.orbitSensitivity, 0.01F, 2.0F);
    result.config.distanceScale = std::clamp(result.config.distanceScale, 0.1, 100'000.0);
    result.config.bodyRadiusScale = std::clamp(result.config.bodyRadiusScale, 1.0, 100'000.0);
    return result;
}

AppConfigResult loadAppConfig(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input) {
        AppConfigResult result;
        result.warnings.push_back("Configuration missing at " + path.string() +
                                  "; using defaults");
        return result;
    }
    try {
        return parseAppConfig(nlohmann::json::parse(input));
    } catch (const std::exception& exception) {
        AppConfigResult result;
        result.warnings.push_back("Invalid configuration at " + path.string() + ": " +
                                  exception.what() + "; using defaults");
        return result;
    }
}

} // namespace solar::app
