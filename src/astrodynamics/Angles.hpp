#pragma once

namespace solar::astrodynamics {

[[nodiscard]] double normalizeAnglePositive(double radians) noexcept;
[[nodiscard]] double normalizeAngleSigned(double radians) noexcept;
[[nodiscard]] double angularDifference(double leftRadians, double rightRadians) noexcept;

} // namespace solar::astrodynamics

