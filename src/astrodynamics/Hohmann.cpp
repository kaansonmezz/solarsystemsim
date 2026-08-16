#include "astrodynamics/Hohmann.hpp"

#include "astrodynamics/OrbitConversions.hpp"

#include <cmath>
#include <numbers>
#include <glm/geometric.hpp>

namespace solar::astrodynamics {

std::optional<HohmannTransfer> calculateHohmannTransfer(
    const double initialRadius, const double finalRadius, const double mu) noexcept
{
    if (!(initialRadius > 0.0) || !(finalRadius > 0.0) || !(mu > 0.0)) {
        return std::nullopt;
    }
    const double transferSemiMajor = 0.5 * (initialRadius + finalRadius);
    const double initialCircular = std::sqrt(mu / initialRadius);
    const double finalCircular = std::sqrt(mu / finalRadius);
    const double transferDeparture = std::sqrt(mu * (2.0 / initialRadius - 1.0 / transferSemiMajor));
    const double transferArrival = std::sqrt(mu * (2.0 / finalRadius - 1.0 / transferSemiMajor));
    const double firstBurn = transferDeparture - initialCircular;
    const double secondBurn = finalCircular - transferArrival;
    return HohmannTransfer{
        initialCircular,
        finalCircular,
        transferDeparture,
        transferArrival,
        firstBurn,
        secondBurn,
        std::abs(firstBurn) + std::abs(secondBurn),
        std::numbers::pi * std::sqrt(
            transferSemiMajor * transferSemiMajor * transferSemiMajor / mu),
        transferSemiMajor,
    };
}

std::optional<CurrentOrbitHohmann> calculateHohmannFromCurrentCircularState(
    const astronomy::StateVector& currentState,
    const double targetRadius,
    const double mu,
    const double circularTolerance) noexcept
{
    if (!(targetRadius > 0.0) || !(circularTolerance >= 0.0)) return std::nullopt;
    const auto elements = stateToOrbitalElements(currentState, mu);
    if (!elements || elements->eccentricity > circularTolerance) return std::nullopt;
    const double currentRadius = glm::length(currentState.positionKm);
    const auto transfer = calculateHohmannTransfer(currentRadius, targetRadius, mu);
    if (!transfer) return std::nullopt;
    return CurrentOrbitHohmann{*transfer, currentRadius, targetRadius};
}

} // namespace solar::astrodynamics
