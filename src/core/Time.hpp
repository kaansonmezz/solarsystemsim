#pragma once

#include "core/Types.hpp"

#include <string>
#include <optional>

namespace solar::core {

struct UtcDateTime final {
    std::string date;
    std::string time;
};

struct UtcComponents final {
    int year{2000};
    unsigned int month{1};
    unsigned int day{1};
    unsigned int hour{};
    unsigned int minute{};
    unsigned int second{};
};

[[nodiscard]] UtcDateTime formatUtc(SimulationTimePoint timePoint);
[[nodiscard]] std::string formatUtcIso8601(SimulationTimePoint timePoint);
[[nodiscard]] std::string formatUtcForSpice(SimulationTimePoint timePoint);
[[nodiscard]] UtcComponents utcComponents(SimulationTimePoint timePoint) noexcept;
[[nodiscard]] std::optional<SimulationTimePoint> makeUtcTimePoint(
    const UtcComponents& components) noexcept;

} // namespace solar::core
