#pragma once

#include <optional>
#include <string_view>

namespace solar::astrodynamics {

enum class OrbitType {
    circular,
    elliptic,
    parabolic,
    hyperbolic,
    degenerate,
};

struct OrbitalElements final {
    double semiMajorAxisKm{0.0};
    double eccentricity{0.0};
    double inclinationRad{0.0};
    double longitudeAscendingNodeRad{0.0};
    double argumentPeriapsisRad{0.0};
    double trueAnomalyRad{0.0};
    double semiLatusRectumKm{0.0};

    [[nodiscard]] OrbitType type() const noexcept;
    [[nodiscard]] double periapsisRadiusKm() const noexcept;
    [[nodiscard]] std::optional<double> apoapsisRadiusKm() const noexcept;
    [[nodiscard]] std::optional<double> periodSeconds(double muKm3PerSec2) const noexcept;
};

[[nodiscard]] std::string_view name(OrbitType type) noexcept;

} // namespace solar::astrodynamics

