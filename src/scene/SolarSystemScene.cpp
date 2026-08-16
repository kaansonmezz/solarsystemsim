#include "scene/SolarSystemScene.hpp"

#include "astronomy/BodyDatabase.hpp"
#include "astronomy/CelestialBody.hpp"
#include "astronomy/Ephemeris.hpp"
#include "astronomy/Orientation.hpp"
#include "astronomy/SpiceContext.hpp"
#include "astrodynamics/OsculatingOrbit.hpp"
#include "core/Logger.hpp"

#include <glm/geometric.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace solar::scene {
namespace {

[[nodiscard]] double orbitSpanDays(const std::string& name)
{
    if (name == "Mercury") return 88.0;
    if (name == "Venus") return 225.0;
    if (name == "Earth") return 365.256;
    if (name == "Moon") return 27.322;
    if (name == "Mars") return 687.0;
    if (name == "Jupiter") return 4'333.0;
    if (name == "Saturn") return 10'759.0;
    if (name == "Uranus") return 30'687.0;
    if (name == "Neptune") return 60'190.0;
    return 0.0;
}

[[nodiscard]] double orbitRefreshDays(const std::string& name)
{
    if (name == "Moon") return 3.0;
    if (name == "Mercury") return 14.0;
    if (name == "Venus" || name == "Earth") return 30.0;
    if (name == "Mars") return 60.0;
    if (name == "Jupiter") return 365.0;
    if (name == "Saturn") return 730.0;
    if (name == "Uranus" || name == "Neptune") return 1'825.0;
    return 30.0;
}

} // namespace

SolarSystemScene::SolarSystemScene(
    const astronomy::BodyDatabase& bodyDatabase,
    astronomy::Ephemeris& ephemeris,
    astronomy::Orientation& orientation,
    astronomy::SpiceContext& spiceContext)
    : m_bodyDatabase(bodyDatabase),
      m_ephemeris(ephemeris),
      m_orientation(orientation),
      m_spiceContext(spiceContext)
{
    m_objects.reserve(m_bodyDatabase.bodies().size());
    for (const auto& body : m_bodyDatabase.bodies()) {
        SceneObject object;
        object.body = &body;
        m_objects.push_back(std::move(object));
    }
    m_referenceFrame.origin = astronomy::OriginMode::sun;
    m_referenceFrame.axes = astronomy::AxisFrame::eclipticJ2000;
    m_referenceFrame.selectedBodyEphemerisId = 399;
}

bool SolarSystemScene::update(const core::SimulationTimePoint simulationTime)
{
    m_currentUtc = simulationTime;
    const auto et = m_ephemeris.ephemerisTime(simulationTime);
    if (!et) {
        m_astronomyReady = false;
        m_statusMessage = m_spiceContext.lastError();
        return false;
    }
    m_currentEt = *et;

    bool allValid = true;
    for (auto& object : m_objects) {
        const auto state = m_ephemeris.getStateAtEt(
            *object.body, 0, m_referenceFrame.axes, m_currentEt);
        object.stateValid = state.has_value() && astronomy::isFinite(*state);
        if (!object.stateValid) {
            allValid = false;
            continue;
        }
        object.barycentricState = *state;
        const auto orientation = m_orientation.bodyFixedToReference(
            *object.body, m_referenceFrame.axes, m_currentEt);
        if (!orientation) {
            object.stateValid = false;
            allValid = false;
            continue;
        }
        object.bodyFixedToReference = *orientation;
    }

    if (!m_osculatingEpochEt || *m_osculatingEpochEt != m_currentEt || !allValid) {
        rebuildOsculatingOrbits();
        if (allValid) {
            m_osculatingEpochEt = m_currentEt;
        } else {
            m_osculatingEpochEt.reset();
        }
    }
    rebuildRenderCoordinates();
    m_astronomyReady = allValid;
    m_statusMessage = allValid ? "Geometric ephemeris state available" : m_spiceContext.lastError();
    if (allValid) {
        if (!m_orbitsInitialized) {
            refreshOrbitCache();
            m_orbitsInitialized = true;
        } else {
            static_cast<void>(refreshStaleOrbitCaches());
        }
    }
    return allValid;
}

void SolarSystemScene::refreshOrbitCache()
{
    for (auto& object : m_objects) {
        refreshOrbit(object);
    }
    ++m_orbitRevision;
}

bool SolarSystemScene::refreshStaleOrbitCaches()
{
    bool refreshed = false;
    for (auto& object : m_objects) {
        if (orbitCacheIsStale(object, m_currentUtc)) {
            refreshOrbit(object);
            refreshed = true;
        }
    }
    if (refreshed) ++m_orbitRevision;
    return refreshed;
}

void SolarSystemScene::refreshOrbit(SceneObject& object)
{
    constexpr int sampleCount = 128;
    constexpr double secondsPerDay = 86'400.0;
    object.orbitReferencePointsKm.clear();
    object.orbitRenderPoints.clear();
    const double spanDays = orbitSpanDays(object.body->name());
    if (spanDays <= 0.0) {
        object.orbitCacheEpoch.reset();
        return;
    }
    const int orbitCenter = object.body->name() == "Moon" ? 399 : 10;
    object.orbitReferencePointsKm.reserve(sampleCount + 1);
    for (int sample = 0; sample <= sampleCount; ++sample) {
        const double fraction = static_cast<double>(sample) / static_cast<double>(sampleCount);
        const double sampleEt = m_currentEt + (fraction - 0.5) * spanDays * secondsPerDay;
        const auto state = m_ephemeris.getStateAtEt(
            *object.body, orbitCenter, m_referenceFrame.axes, sampleEt);
        if (state && astronomy::isFinite(*state)) {
            object.orbitReferencePointsKm.push_back(state->positionKm);
        }
    }
    rebuildOrbitRenderPoints(object);
    object.orbitCacheEpoch = m_currentUtc;
}

bool SolarSystemScene::selectBody(const int naifId) noexcept
{
    if (objectById(naifId) == nullptr) {
        return false;
    }
    m_selectedBodyId = naifId;
    m_referenceFrame.selectedBodyEphemerisId = objectById(naifId)->body->ephemerisId();
    if (m_referenceFrame.origin == astronomy::OriginMode::selectedBody) {
        rebuildRenderCoordinates();
        refreshOrbitCache();
    }
    return true;
}

void SolarSystemScene::setReferenceOrigin(const astronomy::OriginMode origin)
{
    if (m_referenceFrame.origin == origin) return;
    m_referenceFrame.origin = origin;
    rebuildRenderCoordinates();
    if (m_astronomyReady) refreshOrbitCache();
}

void SolarSystemScene::setAxisFrame(const astronomy::AxisFrame axes)
{
    if (m_referenceFrame.axes == axes) return;
    m_referenceFrame.axes = axes;
    m_osculatingEpochEt.reset();
    m_orbitsInitialized = false;
}

void SolarSystemScene::setDistanceScale(const double renderUnitsPerAu)
{
    m_renderTransform.setDistanceScale(renderUnitsPerAu);
    rebuildRenderCoordinates();
    if (m_astronomyReady) refreshOrbitCache();
}

void SolarSystemScene::setBodyRadiusScale(const double multiplier)
{
    m_renderTransform.setBodyRadiusScale(multiplier);
    rebuildRenderCoordinates();
}

const std::vector<SceneObject>& SolarSystemScene::objects() const noexcept { return m_objects; }
std::vector<SceneObject>& SolarSystemScene::objects() noexcept { return m_objects; }
const SceneObject* SolarSystemScene::selectedObject() const noexcept { return objectById(m_selectedBodyId); }
SceneObject* SolarSystemScene::selectedObject() noexcept { return objectById(m_selectedBodyId); }
int SolarSystemScene::selectedBodyId() const noexcept { return m_selectedBodyId; }
const astronomy::ReferenceFrame& SolarSystemScene::referenceFrame() const noexcept { return m_referenceFrame; }
const RenderTransform& SolarSystemScene::renderTransform() const noexcept { return m_renderTransform; }
double SolarSystemScene::currentEphemerisTime() const noexcept { return m_currentEt; }
core::SimulationTimePoint SolarSystemScene::currentUtc() const noexcept { return m_currentUtc; }
bool SolarSystemScene::astronomyReady() const noexcept { return m_astronomyReady; }
const std::string& SolarSystemScene::statusMessage() const noexcept { return m_statusMessage; }
std::uint64_t SolarSystemScene::orbitRevision() const noexcept { return m_orbitRevision; }

bool SolarSystemScene::orbitCacheIsStale(
    const SceneObject& object, const core::SimulationTimePoint time) const noexcept
{
    if (orbitSpanDays(object.body->name()) <= 0.0) return false;
    if (!object.orbitCacheEpoch) return true;
    const double elapsedDays = std::abs(
        std::chrono::duration<double, std::ratio<86'400>>(time - *object.orbitCacheEpoch).count());
    return elapsedDays > orbitRefreshDays(object.body->name());
}

SceneObject* SolarSystemScene::objectById(const int naifId) noexcept
{
    const auto found = std::find_if(m_objects.begin(), m_objects.end(), [&](const SceneObject& object) {
        return object.body->naifId() == naifId;
    });
    return found == m_objects.end() ? nullptr : &*found;
}

const SceneObject* SolarSystemScene::objectById(const int naifId) const noexcept
{
    const auto found = std::find_if(m_objects.begin(), m_objects.end(), [&](const SceneObject& object) {
        return object.body->naifId() == naifId;
    });
    return found == m_objects.end() ? nullptr : &*found;
}

void SolarSystemScene::rebuildRenderCoordinates()
{
    astronomy::StateVector observerState{};
    const int observerId = m_referenceFrame.observerEphemerisId();
    const SceneObject* observerObject = objectById(observerId);
    if (observerObject == nullptr) {
        const auto found = std::find_if(m_objects.begin(), m_objects.end(), [&](const SceneObject& object) {
            return object.body->ephemerisId() == observerId;
        });
        observerObject = found == m_objects.end() ? nullptr : &*found;
    }
    if (observerObject != nullptr && observerObject->stateValid) {
        observerState = observerObject->barycentricState;
    }

    const SceneObject* sun = objectById(10);
    const SceneObject* earth = objectById(399);
    for (auto& object : m_objects) {
        if (!object.stateValid) continue;
        object.referenceState = astronomy::relativeState(object.barycentricState, observerState);
        object.renderPosition = m_renderTransform.toRenderPosition(object.referenceState.positionKm);
        object.renderRadius = m_renderTransform.toRenderRadius(
            object.body->meanRadiusKm(), object.body->renderMetadata().visualRadiusWeight,
            object.body->type() == astronomy::BodyType::star);
        if (sun != nullptr && sun->stateValid) {
            object.distanceFromSunKm = glm::distance(
                object.barycentricState.positionKm, sun->barycentricState.positionKm);
        }
        if (earth != nullptr && earth->stateValid) {
            object.distanceFromEarthKm = glm::distance(
                object.barycentricState.positionKm, earth->barycentricState.positionKm);
        }
        rebuildOrbitRenderPoints(object);
    }

    for (auto& object : m_objects) {
        object.orbitalRenderData.reset();
        if (!object.osculatingOrbit || object.orbitCentralBody == nullptr) continue;
        const SceneObject* center = objectById(object.orbitCentralBody->naifId());
        if (center == nullptr || !center->stateValid) continue;

        const auto& orbit = *object.osculatingOrbit;
        double extentKm = glm::length(orbit.relativeState.positionKm);
        if (const auto apoapsis = orbit.elements.apoapsisRadiusKm()) {
            extentKm = std::max(extentKm, *apoapsis);
        } else if (std::isfinite(orbit.elements.semiMajorAxisKm)) {
            extentKm = std::max(extentKm, std::abs(orbit.elements.semiMajorAxisKm));
        }
        extentKm *= 1.08;
        const float planeRadius = glm::length(
            m_renderTransform.toRenderPosition(orbit.planeBasisU * extentKm));
        if (!(planeRadius > 0.0F) || !std::isfinite(planeRadius)) continue;

        OrbitalRenderData renderData;
        renderData.centralBodyPosition = center->renderPosition;
        renderData.orbitalBasisU = m_renderTransform.toRenderDirection(orbit.planeBasisU);
        renderData.orbitalBasisV = m_renderTransform.toRenderDirection(orbit.planeBasisV);
        renderData.orbitalNormal = m_renderTransform.toRenderDirection(orbit.orbitalNormal);
        renderData.referenceBasisU = m_renderTransform.toRenderDirection({1.0, 0.0, 0.0});
        renderData.referenceBasisV = m_renderTransform.toRenderDirection({0.0, 1.0, 0.0});
        renderData.referenceNormal = m_renderTransform.toRenderDirection({0.0, 0.0, 1.0});
        renderData.planeRadius = planeRadius;
        renderData.markerRadius = std::clamp(planeRadius * 0.015F, 0.025F, 0.8F);
        if (orbit.ascendingNodePositionKm) {
            renderData.ascendingNodePosition = center->renderPosition +
                m_renderTransform.toRenderPosition(*orbit.ascendingNodePositionKm);
        }
        if (orbit.descendingNodePositionKm) {
            renderData.descendingNodePosition = center->renderPosition +
                m_renderTransform.toRenderPosition(*orbit.descendingNodePositionKm);
        }
        object.orbitalRenderData = renderData;
    }
}

void SolarSystemScene::rebuildOsculatingOrbits()
{
    const auto* sun = m_bodyDatabase.find("Sun");
    const auto* earth = m_bodyDatabase.find("Earth");
    for (auto& object : m_objects) {
        object.orbitCentralBody = nullptr;
        object.osculatingOrbit.reset();
        object.orbitalRenderData.reset();
        if (!object.stateValid || object.body->type() == astronomy::BodyType::star) continue;

        const astronomy::CelestialBody* centralBody =
            object.body->type() == astronomy::BodyType::moon ? earth : sun;
        if (centralBody == nullptr) continue;
        const SceneObject* centralObject = objectById(centralBody->naifId());
        if (centralObject == nullptr || !centralObject->stateValid) continue;

        const astronomy::StateVector relativeState = astronomy::relativeState(
            object.barycentricState, centralObject->barycentricState);
        object.orbitCentralBody = centralBody;
        object.osculatingOrbit = astrodynamics::deriveOsculatingOrbit(
            relativeState, centralBody->gravitationalParameterKm3PerSec2());
        if (!object.osculatingOrbit) {
            if (!object.orbitFailureReported) {
                SOLAR_LOG_WARN(
                    "Osculating orbit unavailable for {} relative to {} at ET {:.6f}",
                    object.body->name(), centralBody->name(), m_currentEt);
                object.orbitFailureReported = true;
            }
        } else {
            object.orbitFailureReported = false;
        }
    }
}

void SolarSystemScene::rebuildOrbitRenderPoints(SceneObject& object)
{
    object.orbitRenderPoints.clear();
    object.orbitRenderPoints.reserve(object.orbitReferencePointsKm.size());
    for (const auto& pointKm : object.orbitReferencePointsKm) {
        object.orbitRenderPoints.push_back(m_renderTransform.toRenderPosition(pointKm));
    }
}

} // namespace solar::scene
