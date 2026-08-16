#pragma once

#include "renderer/Mesh.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <glm/vec3.hpp>

namespace solar::renderer { class Shader; }
namespace solar::scene { class SolarSystemScene; }
namespace solar::simulation { struct OrbitVisualizationData; }

namespace solar::renderer {

class TrajectoryRenderer final {
public:
    TrajectoryRenderer();
    ~TrajectoryRenderer();
    TrajectoryRenderer(const TrajectoryRenderer&) = delete;
    TrajectoryRenderer& operator=(const TrajectoryRenderer&) = delete;

    void synchronize(const simulation::OrbitVisualizationData& visualization,
                     const scene::SolarSystemScene& scene);
    void render(const Shader& shader, const simulation::OrbitVisualizationData& visualization,
                const scene::SolarSystemScene& scene) const;

    struct RenderMarker final {
        std::string label;
        glm::vec3 position;
    };
    [[nodiscard]] const std::vector<RenderMarker>& markerPositions() const noexcept;

private:
    std::unique_ptr<Mesh> m_currentOrbit;
    std::unique_ptr<Mesh> m_previewOrbit;
    std::unique_ptr<Mesh> m_basisVectors;
    std::unique_ptr<Mesh> m_markers;
    std::vector<RenderMarker> m_markerPositions;
    std::uint64_t m_geometryRevision{0};
    std::uint64_t m_stateRevision{0};
    std::uint64_t m_sceneOrbitRevision{0};
};

} // namespace solar::renderer
