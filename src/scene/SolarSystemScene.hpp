#pragma once

#include "astronomy/ReferenceFrame.hpp"
#include "core/Types.hpp"
#include "scene/RenderTransform.hpp"
#include "scene/Scene.hpp"
#include "scene/SceneObject.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace solar::astronomy {
class BodyDatabase;
class Ephemeris;
class Orientation;
class SpiceContext;
}

namespace solar::scene {

class SolarSystemScene final : public Scene {
public:
    SolarSystemScene(const astronomy::BodyDatabase& bodyDatabase,
                     astronomy::Ephemeris& ephemeris,
                     astronomy::Orientation& orientation,
                     astronomy::SpiceContext& spiceContext);

    bool update(core::SimulationTimePoint simulationTime);
    void refreshOrbitCache();

    bool selectBody(int naifId) noexcept;
    void setReferenceOrigin(astronomy::OriginMode origin);
    void setAxisFrame(astronomy::AxisFrame axes);
    void setDistanceScale(double renderUnitsPerAu);
    void setBodyRadiusScale(double multiplier);

    [[nodiscard]] const std::vector<SceneObject>& objects() const noexcept;
    [[nodiscard]] std::vector<SceneObject>& objects() noexcept;
    [[nodiscard]] const SceneObject* selectedObject() const noexcept;
    [[nodiscard]] SceneObject* selectedObject() noexcept;
    [[nodiscard]] int selectedBodyId() const noexcept;
    [[nodiscard]] const astronomy::ReferenceFrame& referenceFrame() const noexcept;
    [[nodiscard]] const RenderTransform& renderTransform() const noexcept;
    [[nodiscard]] double currentEphemerisTime() const noexcept;
    [[nodiscard]] core::SimulationTimePoint currentUtc() const noexcept;
    [[nodiscard]] bool astronomyReady() const noexcept;
    [[nodiscard]] const std::string& statusMessage() const noexcept;
    [[nodiscard]] std::uint64_t orbitRevision() const noexcept;
    [[nodiscard]] bool orbitCacheIsStale(const SceneObject& object,
                                         core::SimulationTimePoint time) const noexcept;

    bool showLabels{true};
    bool showOrbits{true};
    bool showGrid{true};
    bool showAxes{true};
    bool showRotationAxis{false};
    bool showEquatorialPlane{false};
    bool showOrbitalPlane{false};
    bool showReferencePlane{false};
    bool showOrbitalNormal{false};
    bool showOrbitNodes{false};
    bool followSelected{false};

private:
    [[nodiscard]] SceneObject* objectById(int naifId) noexcept;
    [[nodiscard]] const SceneObject* objectById(int naifId) const noexcept;
    void rebuildRenderCoordinates();
    void rebuildOsculatingOrbits();
    void rebuildOrbitRenderPoints(SceneObject& object);
    bool refreshStaleOrbitCaches();
    void refreshOrbit(SceneObject& object);

    const astronomy::BodyDatabase& m_bodyDatabase;
    astronomy::Ephemeris& m_ephemeris;
    astronomy::Orientation& m_orientation;
    astronomy::SpiceContext& m_spiceContext;
    std::vector<SceneObject> m_objects;
    astronomy::ReferenceFrame m_referenceFrame;
    RenderTransform m_renderTransform;
    int m_selectedBodyId{399};
    double m_currentEt{0.0};
    core::SimulationTimePoint m_currentUtc{};
    std::string m_statusMessage{"Not initialized"};
    bool m_astronomyReady{false};
    bool m_orbitsInitialized{false};
    std::optional<double> m_osculatingEpochEt;
    std::uint64_t m_orbitRevision{0};
};

} // namespace solar::scene
