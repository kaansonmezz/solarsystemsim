#pragma once

#include <filesystem>

namespace solar::test {

[[nodiscard]] inline std::filesystem::path kernelDirectory()
{
    return std::filesystem::path{SOLAR_TEST_KERNEL_DIR};
}

} // namespace solar::test

