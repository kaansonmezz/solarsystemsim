#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include <glm/vec3.hpp>

namespace solar::renderer {

struct Vertex final {
    glm::vec3 position{0.0F};
    glm::vec3 normal{0.0F, 1.0F, 0.0F};
    glm::vec3 color{1.0F};
};

class Mesh final {
public:
    Mesh(std::span<const Vertex> vertices,
         std::span<const std::uint32_t> indices,
         unsigned int primitiveMode);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw() const noexcept;

    [[nodiscard]] std::size_t vertexCount() const noexcept;
    [[nodiscard]] std::size_t indexCount() const noexcept;

private:
    void release() noexcept;

    unsigned int m_vertexArray{0};
    unsigned int m_vertexBuffer{0};
    unsigned int m_indexBuffer{0};
    unsigned int m_primitiveMode{0};
    int m_vertexCount{0};
    int m_indexCount{0};
};

} // namespace solar::renderer

