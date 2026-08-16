#include "renderer/Shader.hpp"

#include "core/FileSystem.hpp"
#include "core/Logger.hpp"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace solar::renderer {
namespace {

[[nodiscard]] unsigned int compileShader(
    const unsigned int type,
    const std::string& source,
    const std::filesystem::path& path)
{
    const unsigned int shader = glCreateShader(type);
    const char* sourcePointer = source.c_str();
    glShaderSource(shader, 1, &sourcePointer, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE) {
        return shader;
    }

    int logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> log(static_cast<std::size_t>(std::max(logLength, 1)));
    glGetShaderInfoLog(shader, logLength, nullptr, log.data());
    glDeleteShader(shader);

    const std::string message = "Shader compilation failed for " + path.string() + "\n" + log.data();
    SOLAR_LOG_ERROR("{}", message);
    throw std::runtime_error(message);
}

} // namespace

Shader::Shader(
    const std::filesystem::path& vertexPath,
    const std::filesystem::path& fragmentPath)
{
    const std::string vertexSource = core::FileSystem::readTextFile(vertexPath);
    const std::string fragmentSource = core::FileSystem::readTextFile(fragmentPath);
    const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource, vertexPath);
    const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);

    m_program = glCreateProgram();
    glAttachShader(m_program, vertexShader);
    glAttachShader(m_program, fragmentShader);
    glLinkProgram(m_program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int success = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        int logLength = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<std::size_t>(std::max(logLength, 1)));
        glGetProgramInfoLog(m_program, logLength, nullptr, log.data());
        const std::string message = "Shader program link failed:\n" + std::string(log.data());
        glDeleteProgram(m_program);
        m_program = 0;
        SOLAR_LOG_ERROR("{}", message);
        throw std::runtime_error(message);
    }
}

Shader::~Shader()
{
    if (m_program != 0) {
        glDeleteProgram(m_program);
    }
}

Shader::Shader(Shader&& other) noexcept : m_program(std::exchange(other.m_program, 0)) {}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other) {
        if (m_program != 0) {
            glDeleteProgram(m_program);
        }
        m_program = std::exchange(other.m_program, 0);
    }
    return *this;
}

void Shader::bind() const noexcept { glUseProgram(m_program); }
void Shader::unbind() noexcept { glUseProgram(0); }

void Shader::set(const std::string_view name, const bool value) const { set(name, value ? 1 : 0); }
void Shader::set(const std::string_view name, const int value) const { glUniform1i(uniformLocation(name), value); }
void Shader::set(const std::string_view name, const float value) const { glUniform1f(uniformLocation(name), value); }
void Shader::set(const std::string_view name, const glm::vec2& value) const { glUniform2fv(uniformLocation(name), 1, glm::value_ptr(value)); }
void Shader::set(const std::string_view name, const glm::vec3& value) const { glUniform3fv(uniformLocation(name), 1, glm::value_ptr(value)); }
void Shader::set(const std::string_view name, const glm::vec4& value) const { glUniform4fv(uniformLocation(name), 1, glm::value_ptr(value)); }
void Shader::set(const std::string_view name, const glm::mat3& value) const { glUniformMatrix3fv(uniformLocation(name), 1, GL_FALSE, glm::value_ptr(value)); }
void Shader::set(const std::string_view name, const glm::mat4& value) const { glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, glm::value_ptr(value)); }
unsigned int Shader::id() const noexcept { return m_program; }

int Shader::uniformLocation(const std::string_view name) const
{
    const std::string nullTerminated{name};
    return glGetUniformLocation(m_program, nullTerminated.c_str());
}

} // namespace solar::renderer
