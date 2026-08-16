#include "astronomy/SpiceContext.hpp"

#include "core/Logger.hpp"

#include <SpiceUsr.h>
#include <nlohmann/json.hpp>

#include <array>
#include <fstream>
#include <mutex>
#include <sstream>
#include <stdexcept>

namespace solar::astronomy {
namespace {

std::mutex g_spiceMutex;
SpiceContext* g_activeContext = nullptr;

[[nodiscard]] std::string idString(const int id)
{
    return std::to_string(id);
}

} // namespace

SpiceContext::SpiceContext()
{
    std::scoped_lock lock(g_spiceMutex);
    if (g_activeContext != nullptr) {
        throw std::logic_error("Only one SpiceContext may be active because CSPICE state is global");
    }
    g_activeContext = this;
    SpiceChar returnAction[] = "RETURN";
    SpiceChar noPrint[] = "NONE";
    erract_c("SET", 0, returnAction);
    errprt_c("SET", 0, noPrint);
    reset_c();
}

SpiceContext::~SpiceContext()
{
    unloadKernels();
    std::scoped_lock lock(g_spiceMutex);
    if (g_activeContext == this) g_activeContext = nullptr;
}

bool SpiceContext::loadKernel(const std::filesystem::path& path)
{
    if (!std::filesystem::is_regular_file(path)) {
        m_lastError = "Kernel file does not exist: " + path.string();
        SOLAR_LOG_ERROR("SPICE: {}", m_lastError);
        return false;
    }

    std::scoped_lock lock(g_spiceMutex);
    clearLastError();
    const std::string pathString = path.string();
    furnsh_c(pathString.c_str());
    if (captureFailure("loading kernel " + pathString)) {
        return false;
    }
    m_loadedKernels.push_back(std::filesystem::absolute(path));
    SOLAR_LOG_INFO("SPICE kernel loaded: {}", path.filename().string());
    return true;
}

bool SpiceContext::loadKernelManifest(const std::filesystem::path& kernelDirectory)
{
    unloadKernels();
    const auto manifestPath = kernelDirectory / "manifest.json";
    try {
        std::ifstream input(manifestPath);
        if (!input) {
            m_lastError = "Kernel manifest not found: " + manifestPath.string();
            SOLAR_LOG_ERROR("SPICE: {}", m_lastError);
            return false;
        }
        const nlohmann::json manifest = nlohmann::json::parse(input);
        const auto kernels = manifest.at("kernels").get<std::vector<std::string>>();
        if (kernels.empty()) {
            m_lastError = "Kernel manifest contains no kernels";
            SOLAR_LOG_ERROR("SPICE: {}", m_lastError);
            return false;
        }
        for (const auto& kernel : kernels) {
            if (!loadKernel(kernelDirectory / kernel)) {
                const std::string failure = m_lastError;
                unloadKernels();
                m_lastError = failure;
                return false;
            }
        }
        return true;
    } catch (const std::exception& exception) {
        m_lastError = "Invalid kernel manifest " + manifestPath.string() + ": " + exception.what();
        SOLAR_LOG_ERROR("SPICE: {}", m_lastError);
        return false;
    }
}

void SpiceContext::unloadKernels() noexcept
{
    std::scoped_lock lock(g_spiceMutex);
    kclear_c();
    reset_c();
    m_loadedKernels.clear();
}

std::optional<double> SpiceContext::utcToEphemerisTime(const std::string_view utc) const
{
    std::scoped_lock lock(g_spiceMutex);
    clearLastError();
    std::string input{utc};
    if (input.size() > 10 && input[10] == 'T') input[10] = ' ';
    SpiceDouble ephemerisTime = 0.0;
    str2et_c(input.c_str(), &ephemerisTime);
    if (captureFailure("converting UTC '" + input + "' to ET")) {
        return std::nullopt;
    }
    return ephemerisTime;
}

std::optional<StateVector> SpiceContext::getState(
    const int targetNaifId,
    const int observerNaifId,
    const std::string_view frame,
    const double ephemerisTime) const
{
    std::scoped_lock lock(g_spiceMutex);
    clearLastError();
    const std::string target = idString(targetNaifId);
    const std::string observer = idString(observerNaifId);
    const std::string frameName{frame};
    std::array<SpiceDouble, 6> values{};
    SpiceDouble lightTime = 0.0;
    spkezr_c(target.c_str(), ephemerisTime, frameName.c_str(), "NONE",
             observer.c_str(), values.data(), &lightTime);
    if (captureFailure("querying state for target " + target + " relative to " + observer)) {
        return std::nullopt;
    }
    return StateVector{
        {values[0], values[1], values[2]},
        {values[3], values[4], values[5]},
    };
}

std::optional<glm::dmat3> SpiceContext::frameTransform(
    const std::string_view fromFrame,
    const std::string_view toFrame,
    const double ephemerisTime) const
{
    std::scoped_lock lock(g_spiceMutex);
    clearLastError();
    const std::string from{fromFrame};
    const std::string to{toFrame};
    SpiceDouble matrix[3][3]{};
    pxform_c(from.c_str(), to.c_str(), ephemerisTime, matrix);
    if (captureFailure("transforming frame " + from + " to " + to)) {
        return std::nullopt;
    }

    glm::dmat3 result{1.0};
    for (std::size_t row = 0; row < 3; ++row) {
        for (std::size_t column = 0; column < 3; ++column) {
            result[column][row] = matrix[row][column];
        }
    }
    return result;
}

bool SpiceContext::isReady() const noexcept
{
    return m_loadedKernels.size() >= 2;
}

const std::vector<std::filesystem::path>& SpiceContext::loadedKernels() const noexcept
{
    return m_loadedKernels;
}

const std::string& SpiceContext::lastError() const noexcept { return m_lastError; }

std::string SpiceContext::toolkitVersion() const
{
    std::scoped_lock lock(g_spiceMutex);
    return tkvrsn_c("TOOLKIT");
}

bool SpiceContext::captureFailure(const std::string_view operation) const
{
    if (failed_c() == SPICEFALSE) {
        return false;
    }
    std::array<SpiceChar, 2'048> message{};
    getmsg_c("LONG", static_cast<SpiceInt>(message.size()), message.data());
    reset_c();
    m_lastError = std::string(operation) + ": " + message.data();
    SOLAR_LOG_ERROR("SPICE: {}", m_lastError);
    return true;
}

void SpiceContext::clearLastError() const
{
    if (failed_c() != SPICEFALSE) {
        reset_c();
    }
    m_lastError.clear();
}

} // namespace solar::astronomy
