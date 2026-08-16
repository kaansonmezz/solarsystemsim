#include "renderer/OrbitRenderer.hpp"

#include "astronomy/CelestialBody.hpp"
#include "renderer/Mesh.hpp"
#include "renderer/Shader.hpp"
#include "scene/SolarSystemScene.hpp"

#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>

#include <vector>

namespace solar::renderer {

OrbitRenderer::OrbitRenderer() = default;
OrbitRenderer::~OrbitRenderer() = default;

void OrbitRenderer::synchronize(const scene::SolarSystemScene& scene)
{
    if (m_revision == scene.orbitRevision()) return;
    m_orbits.clear();
    for (const auto& object : scene.objects()) {
        if (object.orbitRenderPoints.size() < 2) continue;
        std::vector<Vertex> vertices;
        vertices.reserve(object.orbitRenderPoints.size());
        const glm::vec3 color = object.body->renderMetadata().color * 0.58F;
        for (const auto& point : object.orbitRenderPoints) {
            vertices.push_back({point, {}, color});
        }
        const int centerNaifId = object.body->name() == "Moon" ? 399 : 10;
        m_orbits.push_back({
            std::make_unique<Mesh>(vertices, std::span<const std::uint32_t>{}, GL_LINE_STRIP),
            centerNaifId,
            object.body->naifId(),
        });
    }
    m_revision = scene.orbitRevision();
}

void OrbitRenderer::render(const Shader& shader, const scene::SolarSystemScene& scene) const
{
    shader.bind();
    const auto renderOrbit = [&](const CachedOrbit& orbit) {
        glm::vec3 center{0.0F};
        for (const auto& object : scene.objects()) {
            if (object.body->naifId() == orbit.centerNaifId) {
                center = object.renderPosition;
                break;
            }
        }
        shader.set("uModel", glm::translate(glm::mat4{1.0F}, center));
        orbit.mesh->draw();
    };
    shader.set("uColorScale", glm::vec3{0.62F});
    for (const auto& orbit : m_orbits) {
        if (orbit.bodyNaifId != scene.selectedBodyId()) renderOrbit(orbit);
    }
    shader.set("uColorScale", glm::vec3{1.65F});
    for (const auto& orbit : m_orbits) {
        if (orbit.bodyNaifId == scene.selectedBodyId()) renderOrbit(orbit);
    }
    shader.set("uColorScale", glm::vec3{1.0F});
    shader.set("uModel", glm::mat4{1.0F});
}

} // namespace solar::renderer
