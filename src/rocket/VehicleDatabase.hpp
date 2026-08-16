#pragma once

#include "rocket/Vehicle.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace solar::rocket {

struct VehicleLoadResult final {
    std::optional<Vehicle> vehicle;
    std::vector<std::string> errors;

    [[nodiscard]] bool valid() const noexcept { return vehicle.has_value() && errors.empty(); }
};

[[nodiscard]] nlohmann::json vehicleToJson(const Vehicle& vehicle);
[[nodiscard]] VehicleLoadResult vehicleFromJson(const nlohmann::json& json);
[[nodiscard]] VehicleLoadResult loadVehicle(const std::filesystem::path& path);
[[nodiscard]] ValidationResult saveVehicle(
    const Vehicle& vehicle, const std::filesystem::path& path);

} // namespace solar::rocket
