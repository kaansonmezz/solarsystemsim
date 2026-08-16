#pragma once

#include "renderer/Mesh.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace solar::renderer {

class Shader;
}
namespace solar::scene { class SolarSystemScene; }

namespace solar::renderer {

class OrbitRenderer final {
public:
    OrbitRenderer();
    ~OrbitRenderer();

    OrbitRenderer(const OrbitRenderer&) = delete;
    OrbitRenderer& operator=(const OrbitRenderer&) = delete;

    void synchronize(const scene::SolarSystemScene& scene);
    void render(const Shader& shader, const scene::SolarSystemScene& scene) const;

private:
    struct CachedOrbit final {
        std::unique_ptr<Mesh> mesh;
        int centerNaifId{10};
        int bodyNaifId{0};
    };
    std::vector<CachedOrbit> m_orbits;
    std::uint64_t m_revision{static_cast<std::uint64_t>(-1)};
};

} // namespace solar::renderer
