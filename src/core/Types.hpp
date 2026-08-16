#pragma once

#include <chrono>

namespace solar::core {

using SimulationTimePoint = std::chrono::system_clock::time_point;
using Seconds = std::chrono::duration<double>;

} // namespace solar::core

