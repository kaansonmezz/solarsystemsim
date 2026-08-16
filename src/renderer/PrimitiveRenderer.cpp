#include "renderer/PrimitiveRenderer.hpp"

#include "renderer/Mesh.hpp"
#include "renderer/Shader.hpp"

#include <glad/gl.h>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>

#include <cstdint>
#include <cmath>
#include <vector>

namespace solar::renderer {
namespace {

[[nodiscard]] std::unique_ptr<Mesh> makeSphere(
    const unsigned int segments,
    const unsigned int rings,
    const float radius)
{
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    vertices.reserve(static_cast<std::size_t>((segments + 1) * (rings + 1)));
    indices.reserve(static_cast<std::size_t>(segments * rings * 6));

    for (unsigned int ring = 0; ring <= rings; ++ring) {
        const float v = static_cast<float>(ring) / static_cast<float>(rings);
        const float latitude = v * glm::pi<float>();
        for (unsigned int segment = 0; segment <= segments; ++segment) {
            const float u = static_cast<float>(segment) / static_cast<float>(segments);
            const float longitude = u * glm::two_pi<float>();
            const glm::vec3 normal{
                std::sin(latitude) * std::cos(longitude),
                std::cos(latitude),
                std::sin(latitude) * std::sin(longitude),
            };
            vertices.push_back({normal * radius, normal, {0.34F, 0.68F, 0.95F}});
        }
    }

    const unsigned int rowLength = segments + 1;
    for (unsigned int ring = 0; ring < rings; ++ring) {
        for (unsigned int segment = 0; segment < segments; ++segment) {
            const std::uint32_t first = ring * rowLength + segment;
            const std::uint32_t second = first + rowLength;
            indices.insert(indices.end(), {first, second, first + 1, second, second + 1, first + 1});
        }
    }
    return std::make_unique<Mesh>(vertices, indices, GL_TRIANGLES);
}

[[nodiscard]] std::unique_ptr<Mesh> makeAxes()
{
    constexpr float length = 3.0F;
    const std::vector<Vertex> vertices{
        {{0.0F, 0.01F, 0.0F}, {}, {1.0F, 0.15F, 0.12F}},
        {{length, 0.01F, 0.0F}, {}, {1.0F, 0.15F, 0.12F}},
        {{0.0F, 0.01F, 0.0F}, {}, {0.20F, 1.0F, 0.25F}},
        {{0.0F, length, 0.0F}, {}, {0.20F, 1.0F, 0.25F}},
        {{0.0F, 0.01F, 0.0F}, {}, {0.18F, 0.45F, 1.0F}},
        {{0.0F, 0.01F, length}, {}, {0.18F, 0.45F, 1.0F}},
    };
    return std::make_unique<Mesh>(vertices, std::span<const std::uint32_t>{}, GL_LINES);
}

} // namespace

PrimitiveRenderer::PrimitiveRenderer(
    const unsigned int sphereSegments,
    const unsigned int sphereRings)
    : m_sphere(makeSphere(sphereSegments, sphereRings, 1.0F)), m_axes(makeAxes())
{
}

PrimitiveRenderer::~PrimitiveRenderer() = default;
PrimitiveRenderer::PrimitiveRenderer(PrimitiveRenderer&&) noexcept = default;
PrimitiveRenderer& PrimitiveRenderer::operator=(PrimitiveRenderer&&) noexcept = default;

void PrimitiveRenderer::renderSphere(const Shader& shader) const
{
    shader.bind();
    m_sphere->draw();
}

void PrimitiveRenderer::renderAxes(const Shader& shader) const
{
    shader.bind();
    m_axes->draw();
}

} // namespace solar::renderer
