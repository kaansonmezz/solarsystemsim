#pragma once

#include "rocket/Engine.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace solar::rocket {

class EngineDatabase final {
public:
    [[nodiscard]] const std::vector<Engine>& engines() const noexcept;
    [[nodiscard]] const Engine* find(std::string_view name) const noexcept;

    static EngineDatabase fromEngines(std::vector<Engine> engines);

private:
    std::vector<Engine> m_engines;
};

struct EngineDatabaseResult final {
    EngineDatabase database;
    std::vector<std::string> errors;

    [[nodiscard]] bool valid() const noexcept { return errors.empty(); }
};

[[nodiscard]] nlohmann::json engineToJson(const Engine& engine);
[[nodiscard]] std::optional<Engine> engineFromJson(
    const nlohmann::json& json, std::vector<std::string>& errors,
    std::string_view context = "Engine");
[[nodiscard]] EngineDatabaseResult parseEngineDatabase(const nlohmann::json& json);
[[nodiscard]] EngineDatabaseResult loadEngineDatabase(const std::filesystem::path& path);

} // namespace solar::rocket
