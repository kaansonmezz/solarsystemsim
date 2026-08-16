#pragma once

#include "astronomy/StateVector.hpp"
#include "core/Types.hpp"

namespace solar::simulation {

struct SpacecraftState final {
    astronomy::StateVector relativeState;
    core::SimulationTimePoint time;
};

} // namespace solar::simulation
