#include "astrodynamics/Orbit.hpp"

#include "astrodynamics/OrbitConversions.hpp"
#include "astrodynamics/OrbitPropagation.hpp"
#include "astrodynamics/Tolerances.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace solar::astrodynamics {

Orbit::Orbit(astronomy::StateVector stateAtEpoch,
             const double mu,
             const core::SimulationTimePoint epoch)
    : m_stateAtEpoch(stateAtEpoch), m_muKm3PerSec2(mu), m_epoch(epoch)
{
    if (!(mu > 0.0) || !astronomy::isFinite(stateAtEpoch) ||
        !stateToOrbitalElements(stateAtEpoch, mu)) {
        throw std::invalid_argument("Orbit requires a finite, non-degenerate state and positive mu");
    }
}

const astronomy::StateVector& Orbit::stateAtEpoch() const noexcept { return m_stateAtEpoch; }
double Orbit::muKm3PerSec2() const noexcept { return m_muKm3PerSec2; }
core::SimulationTimePoint Orbit::epoch() const noexcept { return m_epoch; }

std::optional<OrbitalElements> Orbit::elements() const noexcept
{
    return stateToOrbitalElements(m_stateAtEpoch, m_muKm3PerSec2);
}

std::optional<astronomy::StateVector> Orbit::stateAt(
    const core::SimulationTimePoint time) const noexcept
{
    return stateAfter(std::chrono::duration<double>(time - m_epoch).count());
}

std::optional<astronomy::StateVector> Orbit::stateAfter(const double deltaTimeSeconds) const noexcept
{
    return propagateTwoBody(m_stateAtEpoch, m_muKm3PerSec2, deltaTimeSeconds);
}

std::vector<glm::dvec3> Orbit::samplePositions(
    const std::size_t sampleCount, const double hyperbolicMaximumRadiusKm) const
{
    std::vector<glm::dvec3> points;
    if (sampleCount < 2) return points;
    auto sampled = elements();
    if (!sampled) return points;
    double minimumAnomaly = 0.0;
    double maximumAnomaly = 2.0 * std::numbers::pi;
    if (sampled->eccentricity > 1.0) {
        const double asymptote = std::acos(-1.0 / sampled->eccentricity);
        double limit = asymptote - 1.0e-4;
        if (hyperbolicMaximumRadiusKm > sampled->periapsisRadiusKm()) {
            const double cosineLimit = (sampled->semiLatusRectumKm /
                hyperbolicMaximumRadiusKm - 1.0) / sampled->eccentricity;
            limit = std::min(limit, std::acos(std::clamp(cosineLimit, -1.0, 1.0)));
        }
        minimumAnomaly = -limit;
        maximumAnomaly = limit;
    }
    points.reserve(sampleCount);
    for (std::size_t index = 0; index < sampleCount; ++index) {
        const double fraction = static_cast<double>(index) /
                                static_cast<double>(sampleCount - 1);
        sampled->trueAnomalyRad = minimumAnomaly + fraction * (maximumAnomaly - minimumAnomaly);
        if (const auto state = orbitalElementsToState(*sampled, m_muKm3PerSec2)) {
            points.push_back(state->positionKm);
        }
    }
    return points;
}

} // namespace solar::astrodynamics
