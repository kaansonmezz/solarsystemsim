#include "core/FileSystem.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace solar::core {
namespace {

std::filesystem::path g_resourceRoot;

[[nodiscard]] bool hasRuntimeLayout(const std::filesystem::path& candidate)
{
    return std::filesystem::is_directory(candidate / "shaders") &&
           std::filesystem::is_directory(candidate / "config");
}

} // namespace

void FileSystem::initialize(const std::filesystem::path& executablePath)
{
    g_resourceRoot = discoverResourceRoot(executablePath);
}

const std::filesystem::path& FileSystem::resourceRoot()
{
    if (g_resourceRoot.empty()) {
        g_resourceRoot = discoverResourceRoot({});
    }
    return g_resourceRoot;
}

std::filesystem::path FileSystem::shader(const std::string_view filename)
{
    return resourceRoot() / "shaders" / filename;
}

std::filesystem::path FileSystem::config(const std::string_view filename)
{
    return resourceRoot() / "config" / filename;
}

std::filesystem::path FileSystem::asset(const std::string_view relativePath)
{
    return resourceRoot() / "assets" / relativePath;
}

std::string FileSystem::readTextFile(const std::filesystem::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        throw std::runtime_error("Unable to open file: " + path.string());
    }

    std::ostringstream contents;
    contents << stream.rdbuf();
    if (!stream.good() && !stream.eof()) {
        throw std::runtime_error("Unable to read file: " + path.string());
    }
    return contents.str();
}

std::filesystem::path FileSystem::discoverResourceRoot(
    const std::filesystem::path& executablePath)
{
    std::vector<std::filesystem::path> candidates;
    if (!executablePath.empty()) {
        const auto absoluteExecutable = std::filesystem::absolute(executablePath);
        candidates.push_back(absoluteExecutable.parent_path());
        candidates.push_back(absoluteExecutable.parent_path().parent_path());
    }
    candidates.push_back(std::filesystem::current_path());
#ifdef SOLAR_SOURCE_DIR
    candidates.emplace_back(SOLAR_SOURCE_DIR);
#endif

    for (const auto& candidate : candidates) {
        std::error_code error;
        const auto canonical = std::filesystem::weakly_canonical(candidate, error);
        if (!error && hasRuntimeLayout(canonical)) {
            return canonical;
        }
    }

    throw std::runtime_error(
        "Could not locate SOLAR runtime resources (expected shaders/ and config/)");
}

} // namespace solar::core

