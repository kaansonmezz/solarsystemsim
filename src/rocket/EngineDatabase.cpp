#include "rocket/EngineDatabase.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <utility>

namespace solar::rocket {
namespace {

[[nodiscard]] bool equalIgnoringCase(const std::string_view left, const std::string_view right)
{
    return left.size() == right.size() && std::equal(
        left.begin(), left.end(), right.begin(), [](const char a, const char b) {
            return std::tolower(static_cast<unsigned char>(a)) ==
                   std::tolower(static_cast<unsigned char>(b));
        });
}

} // namespace

const std::vector<Engine>& EngineDatabase::engines() const noexcept { return m_engines; }

const Engine* EngineDatabase::find(const std::string_view name) const noexcept
{
    const auto found = std::find_if(m_engines.begin(), m_engines.end(), [&](const Engine& engine) {
        return equalIgnoringCase(engine.name, name);
    });
    return found == m_engines.end() ? nullptr : &*found;
}

EngineDatabase EngineDatabase::fromEngines(std::vector<Engine> engines)
{
    EngineDatabase database;
    database.m_engines = std::move(engines);
    return database;
}

nlohmann::json engineToJson(const Engine& engine)
{
    return {
        {"name", engine.name},
        {"manufacturer", engine.manufacturer},
        {"description", engine.description},
        {"thrustVacuumN", engine.thrustVacuumN},
        {"thrustSeaLevelN", engine.thrustSeaLevelN},
        {"ispVacuumSec", engine.ispVacuumSec},
        {"ispSeaLevelSec", engine.ispSeaLevelSec},
        {"massKg", engine.massKg},
        {"minimumThrottle", engine.minimumThrottle},
        {"maximumThrottle", engine.maximumThrottle},
    };
}

std::optional<Engine> engineFromJson(
    const nlohmann::json& json,
    std::vector<std::string>& errors,
    const std::string_view context)
{
    if (!json.is_object()) {
        errors.push_back(std::string{context} + " must be a JSON object");
        return std::nullopt;
    }
    try {
        Engine engine;
        engine.name = json.at("name").get<std::string>();
        engine.manufacturer = json.value("manufacturer", std::string{});
        engine.description = json.value("description", std::string{});
        engine.thrustVacuumN = json.at("thrustVacuumN").get<double>();
        engine.thrustSeaLevelN = json.at("thrustSeaLevelN").get<double>();
        engine.ispVacuumSec = json.at("ispVacuumSec").get<double>();
        engine.ispSeaLevelSec = json.at("ispSeaLevelSec").get<double>();
        engine.massKg = json.at("massKg").get<double>();
        engine.minimumThrottle = json.value("minimumThrottle", 0.0);
        engine.maximumThrottle = json.value("maximumThrottle", 1.0);
        const auto validation = validateEngine(engine);
        for (const auto& error : validation.errors) {
            errors.push_back(std::string{context} + ": " + error);
        }
        return validation.valid() ? std::optional<Engine>{std::move(engine)} : std::nullopt;
    } catch (const std::exception& exception) {
        errors.push_back(std::string{context} + ": " + exception.what());
        return std::nullopt;
    }
}

EngineDatabaseResult parseEngineDatabase(const nlohmann::json& json)
{
    EngineDatabaseResult result;
    if (!json.is_object() || !json.contains("engines") || !json.at("engines").is_array()) {
        result.errors.push_back("Engine database must contain an 'engines' array");
        return result;
    }
    if (json.value("schemaVersion", 0) != 1) {
        result.errors.push_back("Unsupported or missing engine database schemaVersion");
        return result;
    }
    std::vector<Engine> engines;
    const auto& entries = json.at("engines");
    engines.reserve(entries.size());
    for (std::size_t index = 0; index < entries.size(); ++index) {
        if (auto engine = engineFromJson(
                entries[index], result.errors, "Engine " + std::to_string(index + 1))) {
            if (std::any_of(engines.begin(), engines.end(), [&](const Engine& existing) {
                    return equalIgnoringCase(existing.name, engine->name);
                })) {
                result.errors.push_back("Duplicate engine name: " + engine->name);
            } else {
                engines.push_back(std::move(*engine));
            }
        }
    }
    result.database = EngineDatabase::fromEngines(std::move(engines));
    return result;
}

EngineDatabaseResult loadEngineDatabase(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input) {
        EngineDatabaseResult result;
        result.errors.push_back("Unable to open engine database: " + path.string());
        return result;
    }
    try {
        return parseEngineDatabase(nlohmann::json::parse(input));
    } catch (const std::exception& exception) {
        EngineDatabaseResult result;
        result.errors.push_back("Invalid engine database " + path.string() + ": " +
                                exception.what());
        return result;
    }
}

} // namespace solar::rocket
