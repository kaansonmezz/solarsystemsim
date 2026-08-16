#pragma once

#include <memory>

namespace solar::renderer { class Mesh; class Shader; }
namespace solar::scene { class SolarSystemScene; }
namespace solar::simulation { class Spacecraft; }

namespace solar::renderer {

class SpacecraftRenderer final {
public:
    SpacecraftRenderer();
    ~SpacecraftRenderer();
    SpacecraftRenderer(const SpacecraftRenderer&) = delete;
    SpacecraftRenderer& operator=(const SpacecraftRenderer&) = delete;

    void render(const Shader& shader, const simulation::Spacecraft& spacecraft,
                const scene::SolarSystemScene& scene) const;

private:
    std::unique_ptr<Mesh> m_marker;
};

} // namespace solar::renderer
