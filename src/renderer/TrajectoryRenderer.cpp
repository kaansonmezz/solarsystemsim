#include "renderer/TrajectoryRenderer.hpp"

#include "astronomy/CelestialBody.hpp"
#include "renderer/Shader.hpp"
#include "scene/SolarSystemScene.hpp"
#include "simulation/OrbitVisualization.hpp"

#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>

#include <span>

namespace solar::renderer {
namespace {

[[nodiscard]] glm::vec3 centralBodyPosition(
    const simulation::OrbitVisualizationData& visualization,
    const scene::SolarSystemScene& scene)
{
    for (const auto& object : scene.objects()) {
        if (object.body->ephemerisId() == visualization.centralBodyEphemerisId) {
            return object.renderPosition;
        }
    }
    return {};
}

[[nodiscard]] std::unique_ptr<Mesh> makeTrajectoryMesh(
    const std::vector<glm::dvec3>& physicalPoints,
    const scene::SolarSystemScene& scene,
    const glm::vec3& color,
    const bool dashed)
{
    std::vector<Vertex> vertices;
    vertices.reserve(physicalPoints.size());
    if (dashed) {
        for (std::size_t index = 0; index + 1 < physicalPoints.size(); index += 2) {
            vertices.push_back({scene.renderTransform().toRenderPosition(physicalPoints[index]),
                                {}, color});
            vertices.push_back({scene.renderTransform().toRenderPosition(physicalPoints[index + 1]),
                                {}, color});
        }
    } else {
        for (const auto& point : physicalPoints) {
            vertices.push_back({scene.renderTransform().toRenderPosition(point), {}, color});
        }
    }
    return std::make_unique<Mesh>(vertices, std::span<const std::uint32_t>{},
                                  dashed ? GL_LINES : GL_LINE_STRIP);
}

} // namespace

TrajectoryRenderer::TrajectoryRenderer() = default;
TrajectoryRenderer::~TrajectoryRenderer() = default;

void TrajectoryRenderer::synchronize(
    const simulation::OrbitVisualizationData& visualization,
    const scene::SolarSystemScene& scene)
{
    const bool transformChanged = m_sceneOrbitRevision != scene.orbitRevision();
    const bool geometryChanged = transformChanged ||
        m_geometryRevision != visualization.geometryRevision;
    const bool stateChanged = transformChanged ||
        m_stateRevision != visualization.stateRevision;
    if (!geometryChanged && !stateChanged) return;

    if (geometryChanged) {
        m_currentOrbit = makeTrajectoryMesh(
            visualization.trajectoryPointsKm, scene, {0.20F, 0.95F, 0.92F}, false);
        m_previewOrbit.reset();
        if (!visualization.previewTrajectoryPointsKm.empty()) {
            m_previewOrbit = makeTrajectoryMesh(
                visualization.previewTrajectoryPointsKm, scene, {1.0F, 0.68F, 0.18F}, true);
        }

        m_markerPositions.clear();
        std::vector<Vertex> markerVertices;
        markerVertices.reserve(visualization.markers.size());
        for (const auto& marker : visualization.markers) {
            const glm::vec3 position = scene.renderTransform().toRenderPosition(marker.positionKm);
            m_markerPositions.push_back({simulation::markerLabel(marker.kind), position});
            markerVertices.push_back({position, {}, {1.0F, 1.0F, 0.35F}});
        }
        m_markers = std::make_unique<Mesh>(
            markerVertices, std::span<const std::uint32_t>{}, GL_POINTS);
    }

    if (stateChanged) {
        std::vector<Vertex> basisVertices;
        const auto appendDirection = [&](const glm::dvec3& startKm,
                                         const glm::dvec3& direction,
                                         const glm::vec3& color,
                                         const float length) {
            const glm::vec3 start = scene.renderTransform().toRenderPosition(startKm);
            const glm::vec3 mapped = scene.renderTransform().toRenderPosition(direction * 1.0e6);
            basisVertices.push_back({start, {}, color});
            basisVertices.push_back({start + glm::normalize(mapped) * length, {}, color});
        };
        if (visualization.tangentialDirection) {
            appendDirection(visualization.spacecraftPositionKm, *visualization.tangentialDirection,
                            {0.2F, 1.0F, 0.3F}, 0.7F);
        }
        if (visualization.normalDirection) {
            appendDirection(visualization.spacecraftPositionKm, *visualization.normalDirection,
                            {0.75F, 0.35F, 1.0F}, 0.7F);
        }
        if (visualization.radialDirection) {
            appendDirection(visualization.spacecraftPositionKm, *visualization.radialDirection,
                            {1.0F, 0.35F, 0.2F}, 0.7F);
        }
        if (visualization.maneuverPositionKm && visualization.deltaVelocityDirection) {
            appendDirection(*visualization.maneuverPositionKm,
                            *visualization.deltaVelocityDirection,
                            {1.0F, 0.95F, 0.15F}, 0.95F);
        }
        m_basisVectors = std::make_unique<Mesh>(
            basisVertices, std::span<const std::uint32_t>{}, GL_LINES);
    }
    m_geometryRevision = visualization.geometryRevision;
    m_stateRevision = visualization.stateRevision;
    m_sceneOrbitRevision = scene.orbitRevision();
}

void TrajectoryRenderer::render(
    const Shader& shader,
    const simulation::OrbitVisualizationData& visualization,
    const scene::SolarSystemScene& scene) const
{
    const glm::mat4 model = glm::translate(
        glm::mat4{1.0F}, centralBodyPosition(visualization, scene));
    shader.bind();
    shader.set("uModel", model);
    if (m_currentOrbit) m_currentOrbit->draw();
    if (m_previewOrbit) m_previewOrbit->draw();
    if (m_basisVectors) m_basisVectors->draw();
    if (m_markers) {
        glPointSize(7.0F);
        m_markers->draw();
        glPointSize(1.0F);
    }
    shader.set("uModel", glm::mat4{1.0F});
}

const std::vector<TrajectoryRenderer::RenderMarker>&
TrajectoryRenderer::markerPositions() const noexcept
{
    return m_markerPositions;
}

} // namespace solar::renderer
