#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <glm/vec3.hpp>

namespace solar::simulation {

enum class OrbitMarkerKind { periapsis, apoapsis, ascendingNode, descendingNode };

struct ScientificOrbitMarker final {
    OrbitMarkerKind kind{OrbitMarkerKind::periapsis};
    glm::dvec3 positionKm{};
};

// Central-body-relative double-precision geometry produced by the simulation layer.
// Render scaling and display-coordinate mapping are deliberately absent here.
struct OrbitVisualizationData final {
    std::vector<glm::dvec3> trajectoryPointsKm;
    std::vector<glm::dvec3> previewTrajectoryPointsKm;
    std::vector<ScientificOrbitMarker> markers;
    glm::dvec3 spacecraftPositionKm{};
    std::optional<glm::dvec3> radialDirection;
    std::optional<glm::dvec3> tangentialDirection;
    std::optional<glm::dvec3> normalDirection;
    std::optional<glm::dvec3> maneuverPositionKm;
    std::optional<glm::dvec3> deltaVelocityDirection;
    int centralBodyEphemerisId{};
    std::uint64_t geometryRevision{};
    std::uint64_t stateRevision{};
};

[[nodiscard]] const char* markerLabel(OrbitMarkerKind kind) noexcept;

} // namespace solar::simulation
