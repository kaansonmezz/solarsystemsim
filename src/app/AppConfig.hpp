#pragma once

#include "astronomy/ReferenceFrame.hpp"

#include <filesystem>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace solar::app {

struct AppConfig final {
    int windowWidth{1600};
    int windowHeight{900};
    bool verticalSync{true};
    float fieldOfView{60.0F};
    float orbitSensitivity{0.22F};
    double distanceScale{10.0};
    double bodyRadiusScale{100.0};
    bool showOrbits{true};
    bool showLabels{true};
    std::string kernelDirectory{"assets/kernels"};
    astronomy::OriginMode defaultOrigin{astronomy::OriginMode::sun};
    astronomy::AxisFrame defaultFrame{astronomy::AxisFrame::eclipticJ2000};
};

struct AppConfigResult final {
    AppConfig config;
    std::vector<std::string> warnings;
};

[[nodiscard]] AppConfigResult parseAppConfig(const nlohmann::json& json);
[[nodiscard]] AppConfigResult loadAppConfig(const std::filesystem::path& path);

} // namespace solar::app
