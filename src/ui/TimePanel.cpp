#include "ui/TimePanel.hpp"

#include "app/AppState.hpp"
#include "core/Time.hpp"
#include "core/Logger.hpp"
#include "simulation/SimulationClock.hpp"

#include <imgui.h>

#include <cmath>
#include <algorithm>

namespace solar::ui {

void TimePanel::render(app::AppState& state, simulation::SimulationClock& clock)
{
    if (!state.panels.time) {
        return;
    }
    if (!ImGui::Begin("Simulation Time", &state.panels.time)) {
        ImGui::End();
        return;
    }

    const core::UtcDateTime utc = core::formatUtc(clock.simulationTime());
    if (!m_dateInitialized) {
        m_inputUtc = core::utcComponents(clock.simulationTime());
        m_dateInitialized = true;
    }
    ImGui::TextUnformatted("Simulation Time");
    ImGui::Separator();
    ImGui::Text("UTC date   %s", utc.date.c_str());
    ImGui::Text("UTC time   %s", utc.time.c_str());
    ImGui::Text("Mode       %s", clock.isRealTime() ? "Real Time" : "Simulated");
    ImGui::Text("State      %s", clock.isPaused() ? "Paused" : "Running");
    ImGui::Text("Scale      %.3gx", clock.timeScale());
    ImGui::Text("Sim delta  %.6g s", clock.simulationDeltaSeconds());
    ImGui::Spacing();

    if (ImGui::Button(clock.isPaused() ? "Resume" : "Pause")) {
        clock.togglePaused();
    }
    ImGui::SameLine();
    if (ImGui::Button("Real Time")) {
        clock.returnToRealTime();
    }

    constexpr double scales[]{1.0, 10.0, 100.0, 1'000.0, 10'000.0};
    for (const double scale : scales) {
        ImGui::PushID(static_cast<int>(scale));
        if (ImGui::Button(scale == 1.0 ? "1x" :
                          scale == 10.0 ? "10x" :
                          scale == 100.0 ? "100x" :
                          scale == 1'000.0 ? "1000x" : "10000x")) {
            clock.setTimeScale(scale);
        }
        ImGui::PopID();
        if (scale != scales[4]) {
            ImGui::SameLine();
        }
    }

    if (ImGui::Button("Reverse")) {
        const double magnitude = clock.timeScale() == 0.0 ? 1.0 : std::abs(clock.timeScale());
        clock.setTimeScale(clock.timeScale() > 0.0 ? -magnitude : magnitude);
    }

    double editableScale = clock.timeScale();
    ImGui::SetNextItemWidth(-1.0F);
    if (ImGui::InputDouble("##TimeScale", &editableScale, 1.0, 10.0, "%.6gx")) {
        clock.setTimeScale(editableScale);
    }
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        clock.setTimeScale(editableScale);
    }
    ImGui::TextDisabled("Editable time scale (negative reverses)");
    ImGui::SeparatorText("Set exact UTC");
    int year = m_inputUtc.year;
    int month = static_cast<int>(m_inputUtc.month);
    int day = static_cast<int>(m_inputUtc.day);
    int hour = static_cast<int>(m_inputUtc.hour);
    int minute = static_cast<int>(m_inputUtc.minute);
    int second = static_cast<int>(m_inputUtc.second);
    ImGui::InputInt("Year", &year);
    ImGui::InputInt("Month", &month);
    ImGui::InputInt("Day", &day);
    ImGui::InputInt("Hour", &hour);
    ImGui::InputInt("Minute", &minute);
    ImGui::InputInt("Second", &second);
    m_inputUtc = {year, static_cast<unsigned int>(std::max(month, 0)),
                  static_cast<unsigned int>(std::max(day, 0)),
                  static_cast<unsigned int>(std::max(hour, 0)),
                  static_cast<unsigned int>(std::max(minute, 0)),
                  static_cast<unsigned int>(std::max(second, 0))};
    if (ImGui::Button("Set Simulation Time")) {
        if (const auto selected = core::makeUtcTimePoint(m_inputUtc)) {
            clock.setSimulationTime(*selected);
            m_dateError.clear();
        } else {
            m_dateError = "Invalid UTC calendar date or time";
            SOLAR_LOG_WARN("Rejected invalid UTC date input");
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Current UTC")) {
        m_inputUtc = core::utcComponents(clock.simulationTime());
        m_dateError.clear();
    }
    if (!m_dateError.empty()) {
        ImGui::TextColored({1.0F, 0.35F, 0.3F, 1.0F}, "%s", m_dateError.c_str());
    }
    ImGui::TextDisabled("UTC; invalid dates are rejected. Kernel coverage is shown in Astronomy.");
    ImGui::End();
}

} // namespace solar::ui
