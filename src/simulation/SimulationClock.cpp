#include "simulation/SimulationClock.hpp"

#include <cmath>

namespace solar::simulation {

SimulationClock::SimulationClock(const core::SimulationTimePoint initialTime, WallClock wallClock)
    : m_simulationTime(initialTime), m_wallClock(std::move(wallClock))
{
}

void SimulationClock::advance(const double realDeltaSeconds)
{
    if (m_paused || !std::isfinite(realDeltaSeconds) || realDeltaSeconds <= 0.0) {
        m_simulationDeltaSeconds = 0.0;
        return;
    }

    if (m_realTime) {
        const core::SimulationTimePoint wallTime = m_wallClock();
        m_simulationDeltaSeconds =
            std::chrono::duration<double>(wallTime - m_simulationTime).count();
        m_simulationTime = wallTime;
        return;
    }

    m_simulationDeltaSeconds = realDeltaSeconds * m_timeScale;
    m_simulationTime += std::chrono::duration_cast<core::SimulationTimePoint::duration>(
        core::Seconds{m_simulationDeltaSeconds});
}

void SimulationClock::pause() noexcept
{
    m_paused = true;
    m_simulationDeltaSeconds = 0.0;
}

void SimulationClock::resume() noexcept
{
    if (m_timeScale == 0.0) {
        m_timeScale = 1.0;
    }
    m_paused = false;
}

void SimulationClock::togglePaused() noexcept
{
    m_paused = !m_paused;
    if (m_paused) {
        m_simulationDeltaSeconds = 0.0;
    }
}

void SimulationClock::setTimeScale(const double scale) noexcept
{
    if (!std::isfinite(scale)) {
        return;
    }
    m_timeScale = scale;
    m_realTime = false;
    if (scale != 0.0) {
        m_paused = false;
    } else {
        pause();
    }
}

void SimulationClock::setSimulationTime(const core::SimulationTimePoint timePoint) noexcept
{
    m_simulationTime = timePoint;
    m_realTime = false;
}

void SimulationClock::returnToRealTime()
{
    m_simulationTime = m_wallClock();
    m_timeScale = 1.0;
    m_simulationDeltaSeconds = 0.0;
    m_paused = false;
    m_realTime = true;
}

core::SimulationTimePoint SimulationClock::simulationTime() const noexcept
{
    return m_simulationTime;
}

double SimulationClock::timeScale() const noexcept
{
    return m_timeScale;
}

double SimulationClock::simulationDeltaSeconds() const noexcept
{
    return m_simulationDeltaSeconds;
}

bool SimulationClock::isPaused() const noexcept
{
    return m_paused;
}

bool SimulationClock::isRealTime() const noexcept
{
    return m_realTime;
}

} // namespace solar::simulation
