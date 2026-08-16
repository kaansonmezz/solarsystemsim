#pragma once

#include "core/Types.hpp"

#include <functional>

namespace solar::simulation {

class SimulationClock final {
public:
    using WallClock = std::function<core::SimulationTimePoint()>;

    explicit SimulationClock(
        core::SimulationTimePoint initialTime = std::chrono::system_clock::now(),
        WallClock wallClock = [] { return std::chrono::system_clock::now(); });

    void advance(double realDeltaSeconds);
    void pause() noexcept;
    void resume() noexcept;
    void togglePaused() noexcept;
    void setTimeScale(double scale) noexcept;
    void setSimulationTime(core::SimulationTimePoint timePoint) noexcept;
    void returnToRealTime();

    [[nodiscard]] core::SimulationTimePoint simulationTime() const noexcept;
    [[nodiscard]] double timeScale() const noexcept;
    [[nodiscard]] double simulationDeltaSeconds() const noexcept;
    [[nodiscard]] bool isPaused() const noexcept;
    [[nodiscard]] bool isRealTime() const noexcept;

private:
    core::SimulationTimePoint m_simulationTime;
    WallClock m_wallClock;
    double m_timeScale{1.0};
    double m_simulationDeltaSeconds{0.0};
    bool m_paused{false};
    bool m_realTime{true};
};

} // namespace solar::simulation

