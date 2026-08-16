#include "simulation/SimulationClock.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>

namespace {

using solar::core::SimulationTimePoint;
using solar::simulation::SimulationClock;

[[nodiscard]] double elapsedSeconds(
    const SimulationTimePoint start,
    const SimulationTimePoint end)
{
    return std::chrono::duration<double>(end - start).count();
}

} // namespace

TEST_CASE("SimulationClock progresses at normal speed", "[simulation-clock]")
{
    const SimulationTimePoint epoch{};
    SimulationClock clock(epoch);
    clock.setTimeScale(1.0);
    clock.advance(1.25);

    CHECK(elapsedSeconds(epoch, clock.simulationTime()) == Catch::Approx(1.25));
    CHECK(clock.simulationDeltaSeconds() == Catch::Approx(1.25));
}

TEST_CASE("SimulationClock accelerates deterministically", "[simulation-clock]")
{
    const SimulationTimePoint epoch{};
    SimulationClock clock(epoch);
    clock.setTimeScale(100.0);
    clock.advance(1.0);

    CHECK(elapsedSeconds(epoch, clock.simulationTime()) == Catch::Approx(100.0));
}

TEST_CASE("SimulationClock pause prevents progression", "[simulation-clock]")
{
    const SimulationTimePoint epoch{};
    SimulationClock clock(epoch);
    clock.setTimeScale(10.0);
    clock.pause();
    clock.advance(5.0);

    CHECK(clock.simulationTime() == epoch);
    CHECK(clock.simulationDeltaSeconds() == 0.0);
    CHECK(clock.isPaused());

    clock.resume();
    clock.advance(0.5);
    CHECK(elapsedSeconds(epoch, clock.simulationTime()) == Catch::Approx(5.0));
}

TEST_CASE("SimulationClock supports reverse time", "[simulation-clock]")
{
    const SimulationTimePoint epoch{std::chrono::seconds{1'000}};
    SimulationClock clock(epoch);
    clock.setTimeScale(-100.0);
    clock.advance(2.0);

    CHECK(elapsedSeconds(epoch, clock.simulationTime()) == Catch::Approx(-200.0));
    CHECK(clock.simulationDeltaSeconds() == Catch::Approx(-200.0));
}

TEST_CASE("SimulationClock returns to injected wall-clock time", "[simulation-clock]")
{
    const SimulationTimePoint epoch{};
    const SimulationTimePoint wallTime{std::chrono::seconds{42'000}};
    SimulationClock clock(epoch, [wallTime] { return wallTime; });
    clock.setTimeScale(1'000.0);
    clock.pause();

    clock.returnToRealTime();

    CHECK(clock.simulationTime() == wallTime);
    CHECK(clock.timeScale() == 1.0);
    CHECK_FALSE(clock.isPaused());
    CHECK(clock.isRealTime());
}

TEST_CASE("Real-time mode follows the injected wall clock", "[simulation-clock]")
{
    const SimulationTimePoint initial{std::chrono::seconds{100}};
    SimulationTimePoint wallTime = initial;
    SimulationClock clock(initial, [&wallTime] { return wallTime; });

    wallTime += std::chrono::milliseconds{750};
    clock.advance(0.1);

    CHECK(clock.simulationTime() == wallTime);
    CHECK(clock.simulationDeltaSeconds() == Catch::Approx(0.75));
    CHECK(clock.isRealTime());
}

TEST_CASE("Direct time setting leaves real-time mode", "[simulation-clock]")
{
    SimulationClock clock;
    const SimulationTimePoint selected{std::chrono::seconds{123}};
    clock.setSimulationTime(selected);

    CHECK(clock.simulationTime() == selected);
    CHECK_FALSE(clock.isRealTime());
}
