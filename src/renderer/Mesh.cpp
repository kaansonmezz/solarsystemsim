#include "renderer/Mesh.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

namespace solar::renderer {
namespace {

[[nodiscard]] int checkedCount(const std::size_t count)
{
    if (count > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::length_error("Mesh exceeds OpenGL draw count limit");
    }
    return static_cast<int>(count);
}

} // namespace

Mesh::Mesh(
    const std::span<const Vertex> vertices,
    const std::span<const std::uint32_t> indices,
    const unsigned int primitiveMode)
    : m_primitiveMode(primitiveMode),
      m_vertexCount(checkedCount(vertices.size())),
      m_indexCount(checkedCount(indices.size()))
{
    glCreateVertexArrays(1, &m_vertexArray);
    glCreateBuffers(1, &m_vertexBuffer);
    glNamedBufferData(
        m_vertexBuffer,
        static_cast<GLsizeiptr>(vertices.size_bytes()),
        vertices.data(),
        GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(m_vertexArray, 0, m_vertexBuffer, 0, sizeof(Vertex));

    glEnableVertexArrayAttrib(m_vertexArray, 0);
    glVertexArrayAttribFormat(m_vertexArray, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glVertexArrayAttribBinding(m_vertexArray, 0, 0);
    glEnableVertexArrayAttrib(m_vertexArray, 1);
    glVertexArrayAttribFormat(m_vertexArray, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
    glVertexArrayAttribBinding(m_vertexArray, 1, 0);
    glEnableVertexArrayAttrib(m_vertexArray, 2);
    glVertexArrayAttribFormat(m_vertexArray, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
    glVertexArrayAttribBinding(m_vertexArray, 2, 0);

    if (!indices.empty()) {
        glCreateBuffers(1, &m_indexBuffer);
        glNamedBufferData(
            m_indexBuffer,
            static_cast<GLsizeiptr>(indices.size_bytes()),
            indices.data(),
            GL_STATIC_DRAW);
        glVertexArrayElementBuffer(m_vertexArray, m_indexBuffer);
    }
}

Mesh::~Mesh() { release(); }

Mesh::Mesh(Mesh&& other) noexcept
    : m_vertexArray(std::exchange(other.m_vertexArray, 0)),
      m_vertexBuffer(std::exchange(other.m_vertexBuffer, 0)),
      m_indexBuffer(std::exchange(other.m_indexBuffer, 0)),
      m_primitiveMode(other.m_primitiveMode),
      m_vertexCount(other.m_vertexCount),
      m_indexCount(other.m_indexCount)
{
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other) {
        release();
        m_vertexArray = std::exchange(other.m_vertexArray, 0);
        m_vertexBuffer = std::exchange(other.m_vertexBuffer, 0);
        m_indexBuffer = std::exchange(other.m_indexBuffer, 0);
        m_primitiveMode = other.m_primitiveMode;
        m_vertexCount = other.m_vertexCount;
        m_indexCount = other.m_indexCount;
    }
    return *this;
}

void Mesh::draw() const noexcept
{
    glBindVertexArray(m_vertexArray);
    if (m_indexCount > 0) {
        glDrawElements(m_primitiveMode, m_indexCount, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(m_primitiveMode, 0, m_vertexCount);
    }
    glBindVertexArray(0);
}

std::size_t Mesh::vertexCount() const noexcept { return static_cast<std::size_t>(m_vertexCount); }
std::size_t Mesh::indexCount() const noexcept { return static_cast<std::size_t>(m_indexCount); }

void Mesh::release() noexcept
{
    if (m_indexBuffer != 0) glDeleteBuffers(1, &m_indexBuffer);
    if (m_vertexBuffer != 0) glDeleteBuffers(1, &m_vertexBuffer);
    if (m_vertexArray != 0) glDeleteVertexArrays(1, &m_vertexArray);
    m_indexBuffer = 0;
    m_vertexBuffer = 0;
    m_vertexArray = 0;
}

} // namespace solar::renderer

