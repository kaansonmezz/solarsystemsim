#pragma once

#include "rocket/Engine.hpp"
#include "ui/EnginePanel.hpp"
#include "ui/StagePanel.hpp"

#include <array>
#include <cstddef>
#include <string>

namespace solar::app { class AppState; }
namespace solar::astronomy { class BodyDatabase; }
namespace solar::rocket { class EngineDatabase; struct Vehicle; struct VehicleAnalysis; }

namespace solar::ui {

class RocketPanel final {
public:
    void render(app::AppState& state,
                rocket::Vehicle& vehicle,
                const rocket::EngineDatabase& engineDatabase,
                const astronomy::BodyDatabase& bodyDatabase);

private:
    [[nodiscard]] double referenceGravity(const astronomy::BodyDatabase& bodyDatabase) const;
    void addStage(rocket::Vehicle& vehicle, const rocket::EngineDatabase& engineDatabase);
    void renderStageTable(const rocket::Vehicle& vehicle,
                          const rocket::VehicleAnalysis& analysis);
    void renderCharts(const rocket::Vehicle& vehicle,
                      const rocket::VehicleAnalysis& analysis);

    StagePanel m_stagePanel;
    EnginePanel m_enginePanel;
    std::size_t m_selectedStage{};
    int m_bodyIndex{3};
    rocket::PerformanceEnvironment m_environment{rocket::PerformanceEnvironment::vacuum};
    std::array<char, 512> m_vehiclePath{};
    bool m_pathInitialized{false};
    std::string m_status;
};

} // namespace solar::ui
