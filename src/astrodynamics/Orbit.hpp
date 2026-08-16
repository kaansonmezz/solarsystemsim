#pragma once

#include "astrodynamics/OrbitalElements.hpp"
#include "astronomy/StateVector.hpp"
#include "core/Types.hpp"

#include <optional>
#include <vector>

namespace solar::astrodynamics {

class Orbit final {
public:
    Orbit(astronomy::StateVector stateAtEpoch,
          double muKm3PerSec2,
          core::SimulationTimePoint epoch);

    [[nodiscard]] const astronomy::StateVector& stateAtEpoch() const noexcept;
    [[nodiscard]] double muKm3PerSec2() const noexcept;
    [[nodiscard]] core::SimulationTimePoint epoch() const noexcept;
    [[nodiscard]] std::optional<OrbitalElements> elements() const noexcept;
    [[nodiscard]] std::optional<astronomy::StateVector> stateAt(
        core::SimulationTimePoint time) const noexcept;
    [[nodiscard]] std::optional<astronomy::StateVector> stateAfter(
        double deltaTimeSeconds) const noexcept;
    [[nodiscard]] std::vector<glm::dvec3> samplePositions(
        std::size_t sampleCount = 256,
        double hyperbolicMaximumRadiusKm = 1.0e7) const;

private:
    astronomy::StateVector m_stateAtEpoch;
    double m_muKm3PerSec2{};
    core::SimulationTimePoint m_epoch;
};

} // namespace solar::astrodynamics
