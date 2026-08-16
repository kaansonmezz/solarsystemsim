#include "simulation/OrbitVisualization.hpp"

namespace solar::simulation {

const char* markerLabel(const OrbitMarkerKind kind) noexcept
{
    switch (kind) {
    case OrbitMarkerKind::periapsis: return "Pe";
    case OrbitMarkerKind::apoapsis: return "Ap";
    case OrbitMarkerKind::ascendingNode: return "AN";
    case OrbitMarkerKind::descendingNode: return "DN";
    }
    return "?";
}

} // namespace solar::simulation
