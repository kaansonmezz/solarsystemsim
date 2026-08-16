#pragma once

#include "astrodynamics/Maneuver.hpp"
#include "astrodynamics/Orbit.hpp"
#include "astrodynamics/OrbitalElements.hpp"
#include "simulation/SpacecraftState.hpp"
#include "simulation/OrbitVisualization.hpp"

#include <optional>
#include <string>

namespace solar::simulation {

class Spacecraft final {
public:
    static std::optional<Spacecraft> fromElements(
        std::string name,
        std::string centralBodyName,
        int centralBodyEphemerisId,
        double centralBodyRadiusKm,
        double centralBodyMuKm3PerSec2,
        const astrodynamics::OrbitalElements& elements,
        core::SimulationTimePoint epoch) noexcept;
    static std::optional<Spacecraft> fromApsisAltitudes(
        std::string name,
        std::string centralBodyName,
        int centralBodyEphemerisId,
        double centralBodyRadiusKm,
        double centralBodyMuKm3PerSec2,
        double periapsisAltitudeKm,
        double apoapsisAltitudeKm,
        double inclinationRad,
        double raanRad,
        double argumentPeriapsisRad,
        double trueAnomalyRad,
        core::SimulationTimePoint epoch) noexcept;

    Spacecraft(std::string name,
               std::string centralBodyName,
               int centralBodyEphemerisId,
               double centralBodyRadiusKm,
               astrodynamics::Orbit orbit);

    void update(core::SimulationTimePoint simulationTime) noexcept;
    [[nodiscard]] bool previewManeuver(const astrodynamics::ImpulsiveManeuver& maneuver,
                                       core::SimulationTimePoint burnTime) noexcept;
    [[nodiscard]] bool applyPreview() noexcept;
    void resetPreview() noexcept;
    [[nodiscard]] bool replaceOrbit(const astrodynamics::Orbit& orbit) noexcept;

    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] const std::string& centralBodyName() const noexcept;
    [[nodiscard]] int centralBodyEphemerisId() const noexcept;
    [[nodiscard]] double centralBodyRadiusKm() const noexcept;
    [[nodiscard]] const astrodynamics::Orbit& orbit() const noexcept;
    [[nodiscard]] const SpacecraftState& current() const noexcept;
    [[nodiscard]] const std::optional<astrodynamics::Orbit>& previewOrbit() const noexcept;
    [[nodiscard]] const std::optional<SpacecraftState>& previewState() const noexcept;
    [[nodiscard]] const OrbitVisualizationData& visualizationData() const noexcept;

private:
    std::string m_name;
    std::string m_centralBodyName;
    int m_centralBodyEphemerisId{};
    double m_centralBodyRadiusKm{};
    astrodynamics::Orbit m_orbit;
    SpacecraftState m_current;
    std::optional<astrodynamics::Orbit> m_previewOrbit;
    std::optional<SpacecraftState> m_previewState;
    OrbitVisualizationData m_visualization;

    void rebuildVisualizationGeometry();
    void updateVisualizationState();
};

} // namespace solar::simulation
