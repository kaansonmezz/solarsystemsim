#pragma once

#include "astronomy/StateVector.hpp"

#include <string_view>

namespace solar::astronomy {

enum class AxisFrame {
    j2000,
    eclipticJ2000,
};

enum class OriginMode {
    solarSystemBarycenter,
    sun,
    earth,
    selectedBody,
};

struct ReferenceFrame final {
    OriginMode origin{OriginMode::sun};
    AxisFrame axes{AxisFrame::j2000};
    int selectedBodyEphemerisId{10};

    [[nodiscard]] int observerEphemerisId() const noexcept;
};

[[nodiscard]] std::string_view name(AxisFrame frame) noexcept;
[[nodiscard]] std::string_view name(OriginMode origin) noexcept;
[[nodiscard]] StateVector relativeState(
    const StateVector& targetBarycentric,
    const StateVector& observerBarycentric) noexcept;

} // namespace solar::astronomy

