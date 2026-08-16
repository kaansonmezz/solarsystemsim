#include "rocket/VehicleDatabase.hpp"

#include "rocket/EngineDatabase.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <utility>

namespace solar::rocket {
namespace {

[[nodiscard]] const char* categoryToString(const PropellantCategory category) noexcept
{
    return propellantCategoryName(category);
}

[[nodiscard]] std::optional<PropellantCategory> categoryFromString(const std::string& value)
{
    if (value == "Liquid") return PropellantCategory::liquid;
    if (value == "Solid") return PropellantCategory::solid;
    if (value == "Monopropellant") return PropellantCategory::monopropellant;
    if (value == "Electric") return PropellantCategory::electric;
    if (value == "Generic") return PropellantCategory::generic;
    return std::nullopt;
}

} // namespace

nlohmann::json vehicleToJson(const Vehicle& vehicle)
{
    nlohmann::json stages = nlohmann::json::array();
    for (const auto& stage : vehicle.stages) {
        stages.push_back({
            {"name", stage.name},
            {"structuralMassKg", stage.structuralMassKg},
            {"otherDryMassKg", stage.otherDryMassKg},
            {"interstageMassKg", stage.interstageMassKg},
            {"propellant", {
                {"name", stage.propellant.name},
                {"category", categoryToString(stage.propellant.category)},
                {"massKg", stage.propellant.massKg},
            }},
            {"engine", engineToJson(stage.engine)},
            {"engineCount", stage.engineCount},
            {"throttle", stage.throttle},
        });
    }
    return {
        {"schemaVersion", 1},
        {"name", vehicle.name},
        {"payloadMassKg", vehicle.payloadMassKg},
        {"stages", std::move(stages)},
    };
}

VehicleLoadResult vehicleFromJson(const nlohmann::json& json)
{
    VehicleLoadResult result;
    if (!json.is_object()) {
        result.errors.push_back("Vehicle must be a JSON object");
        return result;
    }
    try {
        if (json.value("schemaVersion", 0) != 1) {
            result.errors.push_back("Unsupported or missing vehicle schemaVersion");
            return result;
        }
        Vehicle vehicle;
        vehicle.name = json.at("name").get<std::string>();
        vehicle.payloadMassKg = json.at("payloadMassKg").get<double>();
        const auto& stages = json.at("stages");
        if (!stages.is_array()) {
            result.errors.push_back("Vehicle 'stages' must be an array");
            return result;
        }
        vehicle.stages.reserve(stages.size());
        for (std::size_t index = 0; index < stages.size(); ++index) {
            const auto& jsonStage = stages[index];
            const std::string context = "Stage " + std::to_string(index + 1);
            if (!jsonStage.is_object()) {
                result.errors.push_back(context + " must be a JSON object");
                continue;
            }
            Stage stage;
            stage.name = jsonStage.at("name").get<std::string>();
            stage.structuralMassKg = jsonStage.at("structuralMassKg").get<double>();
            stage.otherDryMassKg = jsonStage.value("otherDryMassKg", 0.0);
            stage.interstageMassKg = jsonStage.value("interstageMassKg", 0.0);
            const auto& propellant = jsonStage.at("propellant");
            stage.propellant.name = propellant.value("name", std::string{"Generic Propellant"});
            const auto category = categoryFromString(
                propellant.value("category", std::string{"Generic"}));
            if (!category) {
                result.errors.push_back(context + ": invalid propellant category");
            } else {
                stage.propellant.category = *category;
            }
            stage.propellant.massKg = propellant.at("massKg").get<double>();
            auto engine = engineFromJson(jsonStage.at("engine"), result.errors,
                                         context + " engine");
            if (engine) stage.engine = std::move(*engine);
            stage.engineCount = jsonStage.at("engineCount").get<int>();
            stage.throttle = jsonStage.value("throttle", 1.0);
            vehicle.stages.push_back(std::move(stage));
        }
        const auto validation = validateVehicle(vehicle);
        result.errors.insert(result.errors.end(), validation.errors.begin(), validation.errors.end());
        if (result.errors.empty()) result.vehicle = std::move(vehicle);
    } catch (const std::exception& exception) {
        result.errors.push_back(std::string{"Invalid vehicle JSON: "} + exception.what());
    }
    return result;
}

VehicleLoadResult loadVehicle(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input) {
        VehicleLoadResult result;
        result.errors.push_back("Unable to open vehicle: " + path.string());
        return result;
    }
    try {
        return vehicleFromJson(nlohmann::json::parse(input));
    } catch (const std::exception& exception) {
        VehicleLoadResult result;
        result.errors.push_back("Invalid vehicle file " + path.string() + ": " + exception.what());
        return result;
    }
}

ValidationResult saveVehicle(const Vehicle& vehicle, const std::filesystem::path& path)
{
    ValidationResult result = validateVehicle(vehicle);
    if (!result.valid()) return result;
    try {
        if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path());
        std::ofstream output(path);
        if (!output) {
            result.add("Unable to open vehicle for writing: " + path.string());
            return result;
        }
        output << vehicleToJson(vehicle).dump(2) << '\n';
        if (!output) result.add("Unable to finish writing vehicle: " + path.string());
    } catch (const std::exception& exception) {
        result.add("Unable to save vehicle " + path.string() + ": " + exception.what());
    }
    return result;
}

} // namespace solar::rocket
