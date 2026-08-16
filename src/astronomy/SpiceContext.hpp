#pragma once

#include "astronomy/StateVector.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <glm/mat3x3.hpp>

namespace solar::astronomy {

class SpiceContext final {
public:
    // CSPICE owns a process-global kernel pool. SOLAR therefore permits exactly one
    // active application-owned context; destruction clears that shared pool.
    SpiceContext();
    ~SpiceContext();

    SpiceContext(const SpiceContext&) = delete;
    SpiceContext& operator=(const SpiceContext&) = delete;
    SpiceContext(SpiceContext&&) = delete;
    SpiceContext& operator=(SpiceContext&&) = delete;

    bool loadKernel(const std::filesystem::path& path);
    bool loadKernelManifest(const std::filesystem::path& kernelDirectory);
    void unloadKernels() noexcept;

    [[nodiscard]] std::optional<double> utcToEphemerisTime(std::string_view utc) const;
    [[nodiscard]] std::optional<StateVector> getState(
        int targetNaifId,
        int observerNaifId,
        std::string_view frame,
        double ephemerisTime) const;
    [[nodiscard]] std::optional<glm::dmat3> frameTransform(
        std::string_view fromFrame,
        std::string_view toFrame,
        double ephemerisTime) const;

    [[nodiscard]] bool isReady() const noexcept;
    [[nodiscard]] const std::vector<std::filesystem::path>& loadedKernels() const noexcept;
    [[nodiscard]] const std::string& lastError() const noexcept;
    [[nodiscard]] std::string toolkitVersion() const;

private:
    bool captureFailure(std::string_view operation) const;
    void clearLastError() const;

    std::vector<std::filesystem::path> m_loadedKernels;
    mutable std::string m_lastError;
};

} // namespace solar::astronomy
