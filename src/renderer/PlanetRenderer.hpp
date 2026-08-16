#pragma once

#include <memory>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace solar::renderer {

class Mesh;
class Shader;

class PlanetRenderer final {
public:
    PlanetRenderer();
    ~PlanetRenderer();

    PlanetRenderer(const PlanetRenderer&) = delete;
    PlanetRenderer& operator=(const PlanetRenderer&) = delete;

    void renderBody(const Shader& shader,
                    const glm::mat4& model,
                    const glm::vec3& color) const;
    void renderRing(const Shader& shader,
                    const glm::mat4& model,
                    const glm::vec3& color) const;
    void renderRotationAxis(const Shader& shader, const glm::mat4& model) const;

private:
    std::unique_ptr<Mesh> m_sphere;
    std::unique_ptr<Mesh> m_ring;
    std::unique_ptr<Mesh> m_rotationAxis;
};

} // namespace solar::renderer

