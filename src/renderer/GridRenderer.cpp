#include "renderer/GridRenderer.hpp"

#include "renderer/Mesh.hpp"
#include "renderer/Shader.hpp"

#include <glad/gl.h>

#include <vector>

namespace solar::renderer {

GridRenderer::GridRenderer(const int halfExtent, const float spacing)
{
    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<std::size_t>((halfExtent * 2 + 1) * 4));
    const float extent = static_cast<float>(halfExtent) * spacing;

    for (int line = -halfExtent; line <= halfExtent; ++line) {
        const float coordinate = static_cast<float>(line) * spacing;
        const bool major = line % 5 == 0;
        const glm::vec3 color = major ? glm::vec3{0.24F, 0.28F, 0.34F}
                                      : glm::vec3{0.13F, 0.16F, 0.20F};
        vertices.push_back({{-extent, 0.0F, coordinate}, {0.0F, 1.0F, 0.0F}, color});
        vertices.push_back({{extent, 0.0F, coordinate}, {0.0F, 1.0F, 0.0F}, color});
        vertices.push_back({{coordinate, 0.0F, -extent}, {0.0F, 1.0F, 0.0F}, color});
        vertices.push_back({{coordinate, 0.0F, extent}, {0.0F, 1.0F, 0.0F}, color});
    }
    m_grid = std::make_unique<Mesh>(vertices, std::span<const std::uint32_t>{}, GL_LINES);
}

GridRenderer::~GridRenderer() = default;
GridRenderer::GridRenderer(GridRenderer&&) noexcept = default;
GridRenderer& GridRenderer::operator=(GridRenderer&&) noexcept = default;

void GridRenderer::render(const Shader& shader) const
{
    shader.bind();
    m_grid->draw();
}

} // namespace solar::renderer

