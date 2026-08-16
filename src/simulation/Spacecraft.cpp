#include "simulation/Spacecraft.hpp"

#include "astrodynamics/OrbitConversions.hpp"
#include "astrodynamics/ManeuverFrame.hpp"
#include "astrodynamics/Tolerances.hpp"

#include <glm/geometric.hpp>

#include <cmath>
#include <numbers>
#include <utility>

namespace solar::simulation {

std::optional<Spacecraft> Spacecraft::fromElements(
    std::string name,
    std::string centralBodyName,
    const int centralBodyEphemerisId,
    const double centralBodyRadiusKm,
    const double mu,
    const astrodynamics::OrbitalElements& elements,
    const core::SimulationTimePoint epoch) noexcept
{
    const auto state = astrodynamics::orbitalElementsToState(elements, mu);
    if (!state || !(centralBodyRadiusKm > 0.0)) return std::nullopt;
    try {
        return Spacecraft{
            std::move(name), std::move(centralBodyName), centralBodyEphemerisId,
            centralBodyRadiusKm, astrodynamics::Orbit{*state, mu, epoch}};
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<Spacecraft> Spacecraft::fromApsisAltitudes(
    std::string name,
    std::string centralBodyName,
    const int centralBodyEphemerisId,
    const double centralBodyRadiusKm,
    const double mu,
    const double periapsisAltitudeKm,
    const double apoapsisAltitudeKm,
    const double inclinationRad,
    const double raanRad,
    const double argumentPeriapsisRad,
    const double trueAnomalyRad,
    const core::SimulationTimePoint epoch) noexcept
{
    const double periapsis = centralBodyRadiusKm + periapsisAltitudeKm;
    const double apoapsis = centralBodyRadiusKm + apoapsisAltitudeKm;
    if (!(periapsis > 0.0) || apoapsis < periapsis) return std::nullopt;
    const double semiMajor = 0.5 * (periapsis + apoapsis);
    const double eccentricity = (apoapsis - periapsis) / (apoapsis + periapsis);
    const astrodynamics::OrbitalElements elements{
        semiMajor, eccentricity, inclinationRad, raanRad, argumentPeriapsisRad,
        trueAnomalyRad, semiMajor * (1.0 - eccentricity * eccentricity)};
    return fromElements(std::move(name), std::move(centralBodyName), centralBodyEphemerisId,
                        centralBodyRadiusKm, mu, elements, epoch);
}

Spacecraft::Spacecraft(std::string name,
                       std::string centralBodyName,
                       const int centralBodyEphemerisId,
                       const double centralBodyRadiusKm,
                       astrodynamics::Orbit orbit)
    : m_name(std::move(name)),
      m_centralBodyName(std::move(centralBodyName)),
      m_centralBodyEphemerisId(centralBodyEphemerisId),
      m_centralBodyRadiusKm(centralBodyRadiusKm),
      m_orbit(std::move(orbit)),
      m_current{m_orbit.stateAtEpoch(), m_orbit.epoch()}
{
    rebuildVisualizationGeometry();
    updateVisualizationState();
}

void Spacecraft::update(const core::SimulationTimePoint simulationTime) noexcept
{
    if (const auto state = m_orbit.stateAt(simulationTime)) {
        m_current = {*state, simulationTime};
    }
    if (m_previewOrbit) {
        if (const auto state = m_previewOrbit->stateAt(simulationTime)) {
            m_previewState = SpacecraftState{*state, simulationTime};
        }
    }
    updateVisualizationState();
}

bool Spacecraft::previewManeuver(
    const astrodynamics::ImpulsiveManeuver& maneuver,
    const core::SimulationTimePoint burnTime) noexcept
{
    const auto burnState = m_orbit.stateAt(burnTime);
    if (!burnState) return false;
    const auto result = astrodynamics::applyManeuver(*burnState, maneuver);
    if (!result) return false;
    try {
        m_previewOrbit.emplace(*result, m_orbit.muKm3PerSec2(), burnTime);
        m_previewState = SpacecraftState{*result, burnTime};
        rebuildVisualizationGeometry();
        updateVisualizationState();
        return true;
    } catch (...) {
        return false;
    }
}

bool Spacecraft::applyPreview() noexcept
{
    if (!m_previewOrbit) return false;
    m_orbit = *m_previewOrbit;
    m_current = {m_orbit.stateAtEpoch(), m_orbit.epoch()};
    m_previewOrbit.reset();
    m_previewState.reset();
    rebuildVisualizationGeometry();
    updateVisualizationState();
    return true;
}

void Spacecraft::resetPreview() noexcept
{
    if (!m_previewOrbit && !m_previewState) return;
    m_previewOrbit.reset();
    m_previewState.reset();
    rebuildVisualizationGeometry();
    updateVisualizationState();
}

bool Spacecraft::replaceOrbit(const astrodynamics::Orbit& orbit) noexcept
{
    m_orbit = orbit;
    m_current = {m_orbit.stateAtEpoch(), m_orbit.epoch()};
    m_previewOrbit.reset();
    m_previewState.reset();
    rebuildVisualizationGeometry();
    updateVisualizationState();
    return true;
}

const std::string& Spacecraft::name() const noexcept { return m_name; }
const std::string& Spacecraft::centralBodyName() const noexcept { return m_centralBodyName; }
int Spacecraft::centralBodyEphemerisId() const noexcept { return m_centralBodyEphemerisId; }
double Spacecraft::centralBodyRadiusKm() const noexcept { return m_centralBodyRadiusKm; }
const astrodynamics::Orbit& Spacecraft::orbit() const noexcept { return m_orbit; }
const SpacecraftState& Spacecraft::current() const noexcept { return m_current; }
const std::optional<astrodynamics::Orbit>& Spacecraft::previewOrbit() const noexcept
{
    return m_previewOrbit;
}
const std::optional<SpacecraftState>& Spacecraft::previewState() const noexcept
{
    return m_previewState;
}
const OrbitVisualizationData& Spacecraft::visualizationData() const noexcept
{
    return m_visualization;
}

void Spacecraft::rebuildVisualizationGeometry()
{
    m_visualization.trajectoryPointsKm = m_orbit.samplePositions();
    m_visualization.previewTrajectoryPointsKm.clear();
    if (m_previewOrbit) {
        m_visualization.previewTrajectoryPointsKm = m_previewOrbit->samplePositions();
    }
    m_visualization.markers.clear();
    const auto elements = m_orbit.elements();
    if (elements) {
        const auto addMarker = [&](const OrbitMarkerKind kind, const double anomaly) {
            auto markerElements = *elements;
            markerElements.trueAnomalyRad = anomaly;
            if (const auto markerState = astrodynamics::orbitalElementsToState(
                    markerElements, m_orbit.muKm3PerSec2())) {
                m_visualization.markers.push_back({kind, markerState->positionKm});
            }
        };
        addMarker(OrbitMarkerKind::periapsis, 0.0);
        if (elements->eccentricity < 1.0) {
            addMarker(OrbitMarkerKind::apoapsis, std::numbers::pi);
        }
        const bool equatorial = elements->inclinationRad <= astrodynamics::kEquatorialToleranceRad ||
            std::abs(elements->inclinationRad - std::numbers::pi) <=
                astrodynamics::kEquatorialToleranceRad;
        if (!equatorial) {
            addMarker(OrbitMarkerKind::ascendingNode, -elements->argumentPeriapsisRad);
            addMarker(OrbitMarkerKind::descendingNode,
                      std::numbers::pi - elements->argumentPeriapsisRad);
        }
    }
    m_visualization.centralBodyEphemerisId = m_centralBodyEphemerisId;
    ++m_visualization.geometryRevision;
}

void Spacecraft::updateVisualizationState()
{
    m_visualization.spacecraftPositionKm = m_current.relativeState.positionKm;
    m_visualization.radialDirection.reset();
    m_visualization.tangentialDirection.reset();
    m_visualization.normalDirection.reset();
    if (const auto frame = astrodynamics::makeManeuverFrame(m_current.relativeState)) {
        m_visualization.radialDirection = frame->radialOut;
        m_visualization.tangentialDirection = frame->tangential;
        m_visualization.normalDirection = frame->normal;
    }
    m_visualization.maneuverPositionKm.reset();
    m_visualization.deltaVelocityDirection.reset();
    if (m_previewOrbit) {
        const auto before = m_orbit.stateAt(m_previewOrbit->epoch());
        if (before) {
            const glm::dvec3 deltaVelocity =
                m_previewOrbit->stateAtEpoch().velocityKmPerSec - before->velocityKmPerSec;
            if (glm::length(deltaVelocity) > astrodynamics::kVectorTolerance) {
                m_visualization.maneuverPositionKm = before->positionKm;
                m_visualization.deltaVelocityDirection = glm::normalize(deltaVelocity);
            }
        }
    }
    ++m_visualization.stateRevision;
}

} // namespace solar::simulation
