#pragma once

#include "astronomy/StateVector.hpp"
#include "astrodynamics/OrbitalElements.hpp"

#include <optional>

#include <glm/vec3.hpp>

namespace solar::astrodynamics {

// Central-body-relative geometry in the same inertial axes as relativeState.
// All vectors remain double precision and physical until the scene bridge maps them.
struct OsculatingOrbitData final {
    astronomy::StateVector relativeState;
    OrbitalElements elements;
    glm::dvec3 angularMomentumKm2PerSec{};
    glm::dvec3 orbitalNormal{};
    glm::dvec3 planeBasisU{};
    glm::dvec3 planeBasisV{};
    std::optional<glm::dvec3> ascendingNodePositionKm;
    std::optional<glm::dvec3> descendingNodePositionKm;
};

[[nodiscard]] std::optional<OsculatingOrbitData> deriveOsculatingOrbit(
    const astronomy::StateVector& relativeState,
    double centralBodyMuKm3PerSec2) noexcept;

} // namespace solar::astrodynamics
