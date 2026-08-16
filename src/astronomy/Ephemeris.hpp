#pragma once

#include "astronomy/ReferenceFrame.hpp"
#include "core/Types.hpp"

#include <optional>

namespace solar::astronomy {

class CelestialBody;
class SpiceContext;

class Ephemeris final {
public:
    explicit Ephemeris(SpiceContext& spiceContext);

    [[nodiscard]] std::optional<double> ephemerisTime(core::SimulationTimePoint utc) const;
    [[nodiscard]] std::optional<StateVector> getState(
        const CelestialBody& target,
        int observerEphemerisId,
        AxisFrame axes,
        core::SimulationTimePoint utc) const;
    [[nodiscard]] std::optional<StateVector> getStateAtEt(
        const CelestialBody& target,
        int observerEphemerisId,
        AxisFrame axes,
        double ephemerisTime) const;

private:
    SpiceContext& m_spiceContext;
};

} // namespace solar::astronomy

