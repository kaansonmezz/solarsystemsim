#pragma once

#include <memory>

namespace solar::renderer {

class Mesh;
class Shader;

class GridRenderer final {
public:
    GridRenderer(int halfExtent = 20, float spacing = 1.0F);
    ~GridRenderer();

    GridRenderer(const GridRenderer&) = delete;
    GridRenderer& operator=(const GridRenderer&) = delete;
    GridRenderer(GridRenderer&&) noexcept;
    GridRenderer& operator=(GridRenderer&&) noexcept;

    void render(const Shader& shader) const;

private:
    std::unique_ptr<Mesh> m_grid;
};

} // namespace solar::renderer

