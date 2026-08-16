#pragma once

#include <filesystem>
#include <string_view>

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace solar::renderer {

class Shader final {
public:
    Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void bind() const noexcept;
    static void unbind() noexcept;

    void set(std::string_view name, bool value) const;
    void set(std::string_view name, int value) const;
    void set(std::string_view name, float value) const;
    void set(std::string_view name, const glm::vec2& value) const;
    void set(std::string_view name, const glm::vec3& value) const;
    void set(std::string_view name, const glm::vec4& value) const;
    void set(std::string_view name, const glm::mat3& value) const;
    void set(std::string_view name, const glm::mat4& value) const;

    [[nodiscard]] unsigned int id() const noexcept;

private:
    [[nodiscard]] int uniformLocation(std::string_view name) const;
    unsigned int m_program{0};
};

} // namespace solar::renderer

