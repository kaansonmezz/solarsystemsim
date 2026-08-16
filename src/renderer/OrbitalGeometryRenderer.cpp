#include "renderer/OrbitalGeometryRenderer.hpp"

#include "renderer/Mesh.hpp"
#include "renderer/Shader.hpp"
#include "scene/SceneObject.hpp"

#include <glad/gl.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/constants.hpp>

#include <cmath>
#include <span>
#include <vector>

namespace solar::renderer {
namespace {

[[nodiscard]] std::unique_ptr<Mesh> makePlaneGrid(const glm::vec3& color)
{
    constexpr int segments = 96;
    constexpr int rings = 6;
    constexpr int spokes = 12;
    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<std::size_t>((segments * rings + spokes) * 2));
    for (int ring = 1; ring <= rings; ++ring) {
        const float radius = static_cast<float>(ring) / static_cast<float>(rings);
        for (int segment = 0; segment < segments; ++segment) {
            const float angle0 = glm::two_pi<float>() * static_cast<float>(segment) /
                                 static_cast<float>(segments);
            const float angle1 = glm::two_pi<float>() * static_cast<float>(segment + 1) /
                                 static_cast<float>(segments);
            vertices.push_back({{radius * std::cos(angle0), 0.0F, radius * std::sin(angle0)},
                                {0.0F, 1.0F, 0.0F}, color});
            vertices.push_back({{radius * std::cos(angle1), 0.0F, radius * std::sin(angle1)},
                                {0.0F, 1.0F, 0.0F}, color});
        }
    }
    for (int spoke = 0; spoke < spokes; ++spoke) {
        const float angle = glm::two_pi<float>() * static_cast<float>(spoke) /
                            static_cast<float>(spokes);
        vertices.push_back({{0.0F, 0.0F, 0.0F}, {0.0F, 1.0F, 0.0F}, color});
        vertices.push_back({{std::cos(angle), 0.0F, std::sin(angle)},
                            {0.0F, 1.0F, 0.0F}, color});
    }
    return std::make_unique<Mesh>(
        vertices, std::span<const std::uint32_t>{}, GL_LINES);
}

[[nodiscard]] std::unique_ptr<Mesh> makeNormal()
{
    const std::vector<Vertex> vertices{
        {{0.0F, -0.35F, 0.0F}, {}, {0.65F, 0.32F, 0.95F}},
        {{0.0F, 1.0F, 0.0F}, {}, {0.90F, 0.55F, 1.0F}},
    };
    return std::make_unique<Mesh>(vertices, std::span<const std::uint32_t>{}, GL_LINES);
}

[[nodiscard]] std::unique_ptr<Mesh> makeMarker()
{
    const std::vector<Vertex> vertices{
        {{-1.0F, 0.0F, 0.0F}, {}, {1.0F, 0.95F, 0.25F}},
        {{1.0F, 0.0F, 0.0F}, {}, {1.0F, 0.95F, 0.25F}},
        {{0.0F, -1.0F, 0.0F}, {}, {1.0F, 0.95F, 0.25F}},
        {{0.0F, 1.0F, 0.0F}, {}, {1.0F, 0.95F, 0.25F}},
        {{0.0F, 0.0F, -1.0F}, {}, {1.0F, 0.95F, 0.25F}},
        {{0.0F, 0.0F, 1.0F}, {}, {1.0F, 0.95F, 0.25F}},
    };
    return std::make_unique<Mesh>(vertices, std::span<const std::uint32_t>{}, GL_LINES);
}

[[nodiscard]] glm::mat4 basisModel(
    const glm::vec3& center,
    const glm::vec3& basisU,
    const glm::vec3& normal,
    const glm::vec3& basisV,
    const float scale)
{
    glm::mat4 model{1.0F};
    model[0] = glm::vec4{basisU * scale, 0.0F};
    model[1] = glm::vec4{normal * scale, 0.0F};
    model[2] = glm::vec4{basisV * scale, 0.0F};
    model[3] = glm::vec4{center, 1.0F};
    return model;
}

[[nodiscard]] glm::mat4 markerModel(const glm::vec3& position, const float scale)
{
    glm::mat4 model{scale};
    model[3] = glm::vec4{position, 1.0F};
    return model;
}

} // namespace

OrbitalGeometryRenderer::OrbitalGeometryRenderer()
    : m_referenceGrid(makePlaneGrid({0.25F, 0.55F, 0.95F})),
      m_orbitalGrid(makePlaneGrid({1.0F, 0.48F, 0.12F})),
      m_normal(makeNormal()),
      m_marker(makeMarker())
{
}

OrbitalGeometryRenderer::~OrbitalGeometryRenderer() = default;

void OrbitalGeometryRenderer::render(
    const Shader& shader,
    const scene::OrbitalRenderData& data,
    const bool showReferencePlane,
    const bool showOrbitalPlane,
    const bool showOrbitalNormal,
    const bool showNodes) const
{
    shader.bind();
    if (showReferencePlane || showOrbitalPlane) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        if (showReferencePlane) {
            shader.set("uAlpha", 0.34F);
            shader.set("uModel", basisModel(
                data.centralBodyPosition, data.referenceBasisU, data.referenceNormal,
                data.referenceBasisV, data.planeRadius));
            m_referenceGrid->draw();
        }
        if (showOrbitalPlane) {
            shader.set("uAlpha", 0.52F);
            shader.set("uModel", basisModel(
                data.centralBodyPosition, data.orbitalBasisU, data.orbitalNormal,
                data.orbitalBasisV, data.planeRadius));
            m_orbitalGrid->draw();
        }
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    shader.set("uAlpha", 1.0F);
    if (showOrbitalNormal) {
        shader.set("uModel", basisModel(
            data.centralBodyPosition, data.orbitalBasisU, data.orbitalNormal,
            data.orbitalBasisV, data.planeRadius * 0.42F));
        m_normal->draw();
    }
    if (showNodes) {
        if (data.ascendingNodePosition) {
            shader.set("uModel", markerModel(*data.ascendingNodePosition, data.markerRadius));
            m_marker->draw();
        }
        if (data.descendingNodePosition) {
            shader.set("uModel", markerModel(*data.descendingNodePosition, data.markerRadius));
            m_marker->draw();
        }
    }
    shader.set("uModel", glm::mat4{1.0F});
}

} // namespace solar::renderer
