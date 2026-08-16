#include "astrodynamics/OrbitalElements.hpp"

#include "astrodynamics/Tolerances.hpp"

#include <cmath>
#include <numbers>

namespace solar::astrodynamics {

OrbitType OrbitalElements::type() const noexcept
{
    if (!(semiLatusRectumKm > 0.0) || eccentricity < 0.0 || !std::isfinite(eccentricity)) {
        return OrbitType::degenerate;
    }
    if (std::abs(eccentricity - 1.0) <= kParabolicEccentricityTolerance) {
        return OrbitType::parabolic;
    }
    if (eccentricity > 1.0) return OrbitType::hyperbolic;
    if (eccentricity <= kCircularEccentricityTolerance) return OrbitType::circular;
    return OrbitType::elliptic;
}

double OrbitalElements::periapsisRadiusKm() const noexcept
{
    return semiLatusRectumKm / (1.0 + eccentricity);
}

std::optional<double> OrbitalElements::apoapsisRadiusKm() const noexcept
{
    if (eccentricity >= 1.0 || type() == OrbitType::degenerate) return std::nullopt;
    return semiLatusRectumKm / (1.0 - eccentricity);
}

std::optional<double> OrbitalElements::periodSeconds(const double mu) const noexcept
{
    if (!(mu > 0.0) || !(semiMajorAxisKm > 0.0) || eccentricity >= 1.0) {
        return std::nullopt;
    }
    return 2.0 * std::numbers::pi * std::sqrt(
        semiMajorAxisKm * semiMajorAxisKm * semiMajorAxisKm / mu);
}

std::string_view name(const OrbitType type) noexcept
{
    switch (type) {
    case OrbitType::circular: return "Circular";
    case OrbitType::elliptic: return "Elliptic";
    case OrbitType::parabolic: return "Parabolic / near-parabolic";
    case OrbitType::hyperbolic: return "Hyperbolic";
    case OrbitType::degenerate: return "Degenerate / invalid";
    }
    return "Degenerate / invalid";
}

} // namespace solar::astrodynamics

