#pragma once

#include "renderer/Camera.hpp"

#include <memory>
#include <string>
#include <vector>

#include <glm/vec2.hpp>

namespace solar::renderer {

class Framebuffer;
class GridRenderer;
class OrbitRenderer;
class OrbitalGeometryRenderer;
class PlanetRenderer;
class PrimitiveRenderer;
class Shader;
class SpacecraftRenderer;
class TrajectoryRenderer;
}
namespace solar::scene { class SolarSystemScene; }
namespace solar::simulation { class Spacecraft; }

namespace solar::renderer {

struct ScreenLabel final {
    std::string text;
    glm::vec2 normalizedPosition{0.0F};
    glm::vec3 color{1.0F};
};

class Renderer final {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void initialize();
    void renderScene(int width, int height, const scene::SolarSystemScene& scene,
                     const simulation::Spacecraft* spacecraft = nullptr);
    void resize(int width, int height);

    [[nodiscard]] unsigned int viewportTexture() const noexcept;
    [[nodiscard]] Camera& camera() noexcept;
    [[nodiscard]] const Camera& camera() const noexcept;
    [[nodiscard]] int viewportWidth() const noexcept;
    [[nodiscard]] int viewportHeight() const noexcept;
    [[nodiscard]] const std::vector<ScreenLabel>& screenLabels() const noexcept;

private:
    Camera m_camera;
    std::unique_ptr<Framebuffer> m_framebuffer;
    std::unique_ptr<Shader> m_planetShader;
    std::unique_ptr<Shader> m_ringShader;
    std::unique_ptr<Shader> m_lineShader;
    std::unique_ptr<GridRenderer> m_gridRenderer;
    std::unique_ptr<PrimitiveRenderer> m_primitiveRenderer;
    std::unique_ptr<PlanetRenderer> m_planetRenderer;
    std::unique_ptr<OrbitRenderer> m_orbitRenderer;
    std::unique_ptr<OrbitalGeometryRenderer> m_orbitalGeometryRenderer;
    std::unique_ptr<TrajectoryRenderer> m_trajectoryRenderer;
    std::unique_ptr<SpacecraftRenderer> m_spacecraftRenderer;
    std::vector<ScreenLabel> m_screenLabels;
};

} // namespace solar::renderer
