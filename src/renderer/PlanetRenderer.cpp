#include "renderer/PlanetRenderer.hpp"

#include "renderer/Mesh.hpp"
#include "renderer/Shader.hpp"

#include <glad/gl.h>
#include <glm/gtc/constants.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

namespace solar::renderer {
namespace {

[[nodiscard]] std::unique_ptr<Mesh> makeSphere()
{
    constexpr unsigned int segments = 48;
    constexpr unsigned int rings = 24;
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    vertices.reserve((segments + 1) * (rings + 1));
    indices.reserve(segments * rings * 6);
    for (unsigned int ring = 0; ring <= rings; ++ring) {
        const float latitude = static_cast<float>(ring) / static_cast<float>(rings) * glm::pi<float>();
        for (unsigned int segment = 0; segment <= segments; ++segment) {
            const float longitude = static_cast<float>(segment) / static_cast<float>(segments) * glm::two_pi<float>();
            const glm::vec3 normal{
                std::sin(latitude) * std::cos(longitude),
                std::sin(latitude) * std::sin(longitude),
                std::cos(latitude),
            };
            vertices.push_back({normal, normal, glm::vec3{1.0F}});
        }
    }
    const unsigned int row = segments + 1;
    for (unsigned int ring = 0; ring < rings; ++ring) {
        for (unsigned int segment = 0; segment < segments; ++segment) {
            const std::uint32_t first = ring * row + segment;
            const std::uint32_t second = first + row;
            indices.insert(indices.end(), {first, second, first + 1, second, second + 1, first + 1});
        }
    }
    return std::make_unique<Mesh>(vertices, indices, GL_TRIANGLES);
}

[[nodiscard]] std::unique_ptr<Mesh> makeRing()
{
    constexpr unsigned int segments = 128;
    constexpr float innerRadius = 1.25F;
    constexpr float outerRadius = 2.25F;
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    vertices.reserve((segments + 1) * 2);
    indices.reserve(segments * 6);
    for (unsigned int segment = 0; segment <= segments; ++segment) {
        const float angle = static_cast<float>(segment) / static_cast<float>(segments) * glm::two_pi<float>();
        const glm::vec3 direction{std::cos(angle), std::sin(angle), 0.0F};
        vertices.push_back({direction * innerRadius, {0.0F, 0.0F, 1.0F}, glm::vec3{1.0F}});
        vertices.push_back({direction * outerRadius, {0.0F, 0.0F, 1.0F}, glm::vec3{1.0F}});
    }
    for (unsigned int segment = 0; segment < segments; ++segment) {
        const std::uint32_t inner = segment * 2;
        indices.insert(indices.end(), {inner, inner + 1, inner + 2, inner + 1, inner + 3, inner + 2});
    }
    return std::make_unique<Mesh>(vertices, indices, GL_TRIANGLES);
}

[[nodiscard]] std::unique_ptr<Mesh> makeRotationAxis()
{
    const std::vector<Vertex> vertices{
        {{0.0F, 0.0F, -2.2F}, {}, {0.85F, 0.25F, 1.0F}},
        {{0.0F, 0.0F, 2.2F}, {}, {0.85F, 0.25F, 1.0F}},
    };
    return std::make_unique<Mesh>(vertices, std::span<const std::uint32_t>{}, GL_LINES);
}

} // namespace

PlanetRenderer::PlanetRenderer()
    : m_sphere(makeSphere()), m_ring(makeRing()), m_rotationAxis(makeRotationAxis())
{
}

PlanetRenderer::~PlanetRenderer() = default;

void PlanetRenderer::renderBody(
    const Shader& shader, const glm::mat4& model, const glm::vec3& color) const
{
    shader.bind();
    shader.set("uModel", model);
    shader.set("uBaseColor", color);
    m_sphere->draw();
}

void PlanetRenderer::renderRing(
    const Shader& shader, const glm::mat4& model, const glm::vec3& color) const
{
    shader.bind();
    shader.set("uModel", model);
    shader.set("uBaseColor", color);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    m_ring->draw();
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void PlanetRenderer::renderRotationAxis(const Shader& shader, const glm::mat4& model) const
{
    shader.bind();
    shader.set("uModel", model);
    m_rotationAxis->draw();
}

} // namespace solar::renderer
