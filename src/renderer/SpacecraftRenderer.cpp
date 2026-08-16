#include "renderer/SpacecraftRenderer.hpp"

#include "renderer/Mesh.hpp"
#include "renderer/Shader.hpp"
#include "astronomy/CelestialBody.hpp"
#include "scene/SolarSystemScene.hpp"
#include "simulation/Spacecraft.hpp"

#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>

#include <cstdint>
#include <vector>

namespace solar::renderer {

SpacecraftRenderer::SpacecraftRenderer()
{
    const std::vector<Vertex> vertices{
        {{0.0F, 1.0F, 0.0F}, {0.0F, 1.0F, 0.0F}, {}},
        {{1.0F, 0.0F, 0.0F}, {1.0F, 0.0F, 0.0F}, {}},
        {{0.0F, 0.0F, 1.0F}, {0.0F, 0.0F, 1.0F}, {}},
        {{-1.0F, 0.0F, 0.0F}, {-1.0F, 0.0F, 0.0F}, {}},
        {{0.0F, 0.0F, -1.0F}, {0.0F, 0.0F, -1.0F}, {}},
        {{0.0F, -1.0F, 0.0F}, {0.0F, -1.0F, 0.0F}, {}},
    };
    const std::vector<std::uint32_t> indices{
        0,1,2, 0,2,3, 0,3,4, 0,4,1,
        5,2,1, 5,3,2, 5,4,3, 5,1,4,
    };
    m_marker = std::make_unique<Mesh>(vertices, indices, GL_TRIANGLES);
}

SpacecraftRenderer::~SpacecraftRenderer() = default;

void SpacecraftRenderer::render(
    const Shader& shader, const simulation::Spacecraft& spacecraft,
    const scene::SolarSystemScene& scene) const
{
    glm::vec3 center{};
    for (const auto& object : scene.objects()) {
        if (object.body->ephemerisId() == spacecraft.centralBodyEphemerisId()) {
            center = object.renderPosition;
            break;
        }
    }
    const glm::vec3 relative = scene.renderTransform().toRenderPosition(
        spacecraft.current().relativeState.positionKm);
    const glm::mat4 model = glm::translate(glm::mat4{1.0F}, center + relative) *
                            glm::scale(glm::mat4{1.0F}, glm::vec3{0.09F});
    shader.bind();
    shader.set("uModel", model);
    shader.set("uBaseColor", glm::vec3{0.25F, 1.0F, 0.9F});
    shader.set("uEmissive", true);
    m_marker->draw();
}

} // namespace solar::renderer
