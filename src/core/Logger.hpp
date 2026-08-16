#pragma once

#include <memory>
#include <string>
#include <vector>

#include <spdlog/logger.h>

namespace solar::core {

enum class LogLevel { info, warning, error };

struct LogMessage final {
    LogLevel level{LogLevel::info};
    std::string text;
};

class Logger final {
public:
    static void initialize();
    static void shutdown();
    [[nodiscard]] static std::shared_ptr<spdlog::logger> get();
    [[nodiscard]] static std::vector<LogMessage> consoleMessages();
    static void clearConsole();
};

} // namespace solar::core

#define SOLAR_LOG_INFO(...) ::solar::core::Logger::get()->info(__VA_ARGS__)
#define SOLAR_LOG_WARN(...) ::solar::core::Logger::get()->warn(__VA_ARGS__)
#define SOLAR_LOG_ERROR(...) ::solar::core::Logger::get()->error(__VA_ARGS__)
