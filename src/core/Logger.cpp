#include "core/Logger.hpp"

#include "core/Constants.hpp"

#include <spdlog/logger.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <deque>
#include <mutex>

namespace solar::core {
namespace {

std::shared_ptr<spdlog::logger> g_logger;
std::mutex g_messagesMutex;
std::deque<LogMessage> g_messages;

template <typename Mutex>
class ConsoleSink final : public spdlog::sinks::base_sink<Mutex> {
protected:
    void sink_it_(const spdlog::details::log_msg& message) override
    {
        spdlog::memory_buf_t formatted;
        this->formatter_->format(message, formatted);

        LogLevel level = LogLevel::info;
        if (message.level >= spdlog::level::err) {
            level = LogLevel::error;
        } else if (message.level >= spdlog::level::warn) {
            level = LogLevel::warning;
        }

        std::scoped_lock lock(g_messagesMutex);
        g_messages.push_back({level, fmt::to_string(formatted)});
        while (g_messages.size() > kMaximumConsoleMessages) {
            g_messages.pop_front();
        }
    }

    void flush_() override {}
};

} // namespace

void Logger::initialize()
{
    if (g_logger) {
        return;
    }

    auto terminalSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto consoleSink = std::make_shared<ConsoleSink<std::mutex>>();
    terminalSink->set_pattern("[%H:%M:%S] [%^%l%$] %v");
    consoleSink->set_pattern("[%H:%M:%S] [%l] %v");

    g_logger = std::make_shared<spdlog::logger>(
        "SOLAR", spdlog::sinks_init_list{terminalSink, consoleSink});
    g_logger->set_level(spdlog::level::debug);
    g_logger->flush_on(spdlog::level::warn);
    spdlog::register_logger(g_logger);
}

void Logger::shutdown()
{
    if (g_logger) {
        g_logger->flush();
        spdlog::drop(g_logger->name());
        g_logger.reset();
    }
}

std::shared_ptr<spdlog::logger> Logger::get()
{
    if (!g_logger) {
        initialize();
    }
    return g_logger;
}

std::vector<LogMessage> Logger::consoleMessages()
{
    std::scoped_lock lock(g_messagesMutex);
    return {g_messages.begin(), g_messages.end()};
}

void Logger::clearConsole()
{
    std::scoped_lock lock(g_messagesMutex);
    g_messages.clear();
}

} // namespace solar::core
