#include "astronomy/Ephemeris.hpp"

#include "astronomy/CelestialBody.hpp"
#include "astronomy/SpiceContext.hpp"
#include "core/Time.hpp"

namespace solar::astronomy {

Ephemeris::Ephemeris(SpiceContext& spiceContext) : m_spiceContext(spiceContext) {}

std::optional<double> Ephemeris::ephemerisTime(const core::SimulationTimePoint utc) const
{
    return m_spiceContext.utcToEphemerisTime(core::formatUtcForSpice(utc));
}

std::optional<StateVector> Ephemeris::getState(
    const CelestialBody& target,
    const int observerEphemerisId,
    const AxisFrame axes,
    const core::SimulationTimePoint utc) const
{
    const auto et = ephemerisTime(utc);
    if (!et) {
        return std::nullopt;
    }
    return getStateAtEt(target, observerEphemerisId, axes, *et);
}

std::optional<StateVector> Ephemeris::getStateAtEt(
    const CelestialBody& target,
    const int observerEphemerisId,
    const AxisFrame axes,
    const double ephemerisTimeValue) const
{
    return m_spiceContext.getState(
        target.ephemerisId(), observerEphemerisId, name(axes), ephemerisTimeValue);
}

} // namespace solar::astronomy
