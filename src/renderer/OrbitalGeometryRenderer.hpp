#pragma once

#include <memory>

namespace solar::renderer {

class Mesh;
class Shader;
}
namespace solar::scene { struct OrbitalRenderData; }

namespace solar::renderer {

class OrbitalGeometryRenderer final {
public:
    OrbitalGeometryRenderer();
    ~OrbitalGeometryRenderer();

    OrbitalGeometryRenderer(const OrbitalGeometryRenderer&) = delete;
    OrbitalGeometryRenderer& operator=(const OrbitalGeometryRenderer&) = delete;

    void render(const Shader& shader,
                const scene::OrbitalRenderData& data,
                bool showReferencePlane,
                bool showOrbitalPlane,
                bool showOrbitalNormal,
                bool showNodes) const;

private:
    std::unique_ptr<Mesh> m_referenceGrid;
    std::unique_ptr<Mesh> m_orbitalGrid;
    std::unique_ptr<Mesh> m_normal;
    std::unique_ptr<Mesh> m_marker;
};

} // namespace solar::renderer
