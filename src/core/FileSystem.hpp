#pragma once

#include <filesystem>
#include <string_view>

namespace solar::core {

class FileSystem final {
public:
    static void initialize(const std::filesystem::path& executablePath);

    [[nodiscard]] static const std::filesystem::path& resourceRoot();
    [[nodiscard]] static std::filesystem::path shader(std::string_view filename);
    [[nodiscard]] static std::filesystem::path config(std::string_view filename);
    [[nodiscard]] static std::filesystem::path asset(std::string_view relativePath);
    [[nodiscard]] static std::string readTextFile(const std::filesystem::path& path);

private:
    [[nodiscard]] static std::filesystem::path discoverResourceRoot(
        const std::filesystem::path& executablePath);
};

} // namespace solar::core

