#pragma once

#include <memory>

namespace solar::renderer {

class Mesh;
class Shader;

class PrimitiveRenderer final {
public:
    PrimitiveRenderer(unsigned int sphereSegments = 48, unsigned int sphereRings = 24);
    ~PrimitiveRenderer();

    PrimitiveRenderer(const PrimitiveRenderer&) = delete;
    PrimitiveRenderer& operator=(const PrimitiveRenderer&) = delete;
    PrimitiveRenderer(PrimitiveRenderer&&) noexcept;
    PrimitiveRenderer& operator=(PrimitiveRenderer&&) noexcept;

    void renderSphere(const Shader& shader) const;
    void renderAxes(const Shader& shader) const;

private:
    std::unique_ptr<Mesh> m_sphere;
    std::unique_ptr<Mesh> m_axes;
};

} // namespace solar::renderer

