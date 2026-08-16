#pragma once

namespace solar::renderer {

class Framebuffer final {
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    void bind() const noexcept;
    static void unbind() noexcept;
    void resize(int width, int height);

    [[nodiscard]] unsigned int colorTexture() const noexcept;
    [[nodiscard]] int width() const noexcept;
    [[nodiscard]] int height() const noexcept;

private:
    void create();
    void release() noexcept;

    unsigned int m_framebuffer{0};
    unsigned int m_colorTexture{0};
    unsigned int m_depthRenderbuffer{0};
    int m_width{1};
    int m_height{1};
};

} // namespace solar::renderer

