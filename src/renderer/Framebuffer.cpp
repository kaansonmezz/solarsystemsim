#include "renderer/Framebuffer.hpp"

#include "core/Logger.hpp"

#include <glad/gl.h>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace solar::renderer {

Framebuffer::Framebuffer(const int width, const int height)
    : m_width(std::max(width, 1)), m_height(std::max(height, 1))
{
    create();
}

Framebuffer::~Framebuffer() { release(); }

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : m_framebuffer(std::exchange(other.m_framebuffer, 0)),
      m_colorTexture(std::exchange(other.m_colorTexture, 0)),
      m_depthRenderbuffer(std::exchange(other.m_depthRenderbuffer, 0)),
      m_width(other.m_width),
      m_height(other.m_height)
{
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this != &other) {
        release();
        m_framebuffer = std::exchange(other.m_framebuffer, 0);
        m_colorTexture = std::exchange(other.m_colorTexture, 0);
        m_depthRenderbuffer = std::exchange(other.m_depthRenderbuffer, 0);
        m_width = other.m_width;
        m_height = other.m_height;
    }
    return *this;
}

void Framebuffer::bind() const noexcept { glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer); }
void Framebuffer::unbind() noexcept { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void Framebuffer::resize(const int width, const int height)
{
    const int safeWidth = std::max(width, 1);
    const int safeHeight = std::max(height, 1);
    if (safeWidth == m_width && safeHeight == m_height) {
        return;
    }

    m_width = safeWidth;
    m_height = safeHeight;
    release();
    create();
    SOLAR_LOG_INFO("Framebuffer resized to {}x{}", m_width, m_height);
}

unsigned int Framebuffer::colorTexture() const noexcept { return m_colorTexture; }
int Framebuffer::width() const noexcept { return m_width; }
int Framebuffer::height() const noexcept { return m_height; }

void Framebuffer::create()
{
    glCreateFramebuffers(1, &m_framebuffer);
    glCreateTextures(GL_TEXTURE_2D, 1, &m_colorTexture);
    glTextureStorage2D(m_colorTexture, 1, GL_RGBA8, m_width, m_height);
    glTextureParameteri(m_colorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_colorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_colorTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_colorTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glNamedFramebufferTexture(m_framebuffer, GL_COLOR_ATTACHMENT0, m_colorTexture, 0);

    glCreateRenderbuffers(1, &m_depthRenderbuffer);
    glNamedRenderbufferStorage(m_depthRenderbuffer, GL_DEPTH24_STENCIL8, m_width, m_height);
    glNamedFramebufferRenderbuffer(
        m_framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);

    if (glCheckNamedFramebufferStatus(m_framebuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        release();
        throw std::runtime_error("Failed to create a complete OpenGL framebuffer");
    }
}

void Framebuffer::release() noexcept
{
    if (m_depthRenderbuffer != 0) glDeleteRenderbuffers(1, &m_depthRenderbuffer);
    if (m_colorTexture != 0) glDeleteTextures(1, &m_colorTexture);
    if (m_framebuffer != 0) glDeleteFramebuffers(1, &m_framebuffer);
    m_depthRenderbuffer = 0;
    m_colorTexture = 0;
    m_framebuffer = 0;
}

} // namespace solar::renderer
