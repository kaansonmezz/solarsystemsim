#include "astrodynamics/Angles.hpp"

#include <cmath>
#include <numbers>

namespace solar::astrodynamics {

double normalizeAnglePositive(const double radians) noexcept
{
    constexpr double twoPi = 2.0 * std::numbers::pi;
    double normalized = std::fmod(radians, twoPi);
    if (normalized < 0.0) normalized += twoPi;
    return normalized;
}

double normalizeAngleSigned(const double radians) noexcept
{
    double normalized = normalizeAnglePositive(radians);
    if (normalized > std::numbers::pi) normalized -= 2.0 * std::numbers::pi;
    return normalized;
}

double angularDifference(const double leftRadians, const double rightRadians) noexcept
{
    return normalizeAngleSigned(leftRadians - rightRadians);
}

} // namespace solar::astrodynamics

