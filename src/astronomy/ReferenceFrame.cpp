#include "astronomy/ReferenceFrame.hpp"

#include <cmath>

namespace solar::astronomy {

bool isFinite(const StateVector& state) noexcept
{
    return std::isfinite(state.positionKm.x) && std::isfinite(state.positionKm.y) &&
           std::isfinite(state.positionKm.z) && std::isfinite(state.velocityKmPerSec.x) &&
           std::isfinite(state.velocityKmPerSec.y) && std::isfinite(state.velocityKmPerSec.z);
}

int ReferenceFrame::observerEphemerisId() const noexcept
{
    switch (origin) {
    case OriginMode::solarSystemBarycenter: return 0;
    case OriginMode::sun: return 10;
    case OriginMode::earth: return 399;
    case OriginMode::selectedBody: return selectedBodyEphemerisId;
    }
    return 10;
}

std::string_view name(const AxisFrame frame) noexcept
{
    switch (frame) {
    case AxisFrame::j2000: return "J2000";
    case AxisFrame::eclipticJ2000: return "ECLIPJ2000";
    }
    return "J2000";
}

std::string_view name(const OriginMode origin) noexcept
{
    switch (origin) {
    case OriginMode::solarSystemBarycenter: return "Solar System Barycenter";
    case OriginMode::sun: return "Sun";
    case OriginMode::earth: return "Earth";
    case OriginMode::selectedBody: return "Selected Body";
    }
    return "Sun";
}

StateVector relativeState(
    const StateVector& targetBarycentric,
    const StateVector& observerBarycentric) noexcept
{
    return {
        targetBarycentric.positionKm - observerBarycentric.positionKm,
        targetBarycentric.velocityKmPerSec - observerBarycentric.velocityKmPerSec,
    };
}

} // namespace solar::astronomy

