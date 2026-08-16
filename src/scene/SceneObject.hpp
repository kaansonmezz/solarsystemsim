#pragma once

#include "astronomy/StateVector.hpp"
#include "astrodynamics/OsculatingOrbit.hpp"
#include "core/Types.hpp"

#include <optional>
#include <vector>

#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>

namespace solar::astronomy { class CelestialBody; }

namespace solar::scene {

struct OrbitalRenderData final {
    glm::vec3 centralBodyPosition{0.0F};
    glm::vec3 orbitalBasisU{1.0F, 0.0F, 0.0F};
    glm::vec3 orbitalBasisV{0.0F, 0.0F, -1.0F};
    glm::vec3 orbitalNormal{0.0F, 1.0F, 0.0F};
    glm::vec3 referenceBasisU{1.0F, 0.0F, 0.0F};
    glm::vec3 referenceBasisV{0.0F, 0.0F, -1.0F};
    glm::vec3 referenceNormal{0.0F, 1.0F, 0.0F};
    std::optional<glm::vec3> ascendingNodePosition;
    std::optional<glm::vec3> descendingNodePosition;
    float planeRadius{1.0F};
    float markerRadius{0.03F};
};

struct SceneObject final {
    const astronomy::CelestialBody* body{nullptr};
    astronomy::StateVector barycentricState;
    astronomy::StateVector referenceState;
    glm::dmat3 bodyFixedToReference{1.0};
    const astronomy::CelestialBody* orbitCentralBody{nullptr};
    std::optional<astrodynamics::OsculatingOrbitData> osculatingOrbit;
    std::optional<OrbitalRenderData> orbitalRenderData;
    glm::vec3 renderPosition{0.0F};
    float renderRadius{0.1F};
    double distanceFromSunKm{0.0};
    double distanceFromEarthKm{0.0};
    bool visible{true};
    bool stateValid{false};
    bool orbitFailureReported{false};
    std::vector<glm::dvec3> orbitReferencePointsKm;
    std::vector<glm::vec3> orbitRenderPoints;
    std::optional<core::SimulationTimePoint> orbitCacheEpoch;
};

} // namespace solar::scene
