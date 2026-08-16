#include "app/Application.hpp"

#include "core/Logger.hpp"

#include <exception>

int main(int argc, char** argv)
{
    try {
        solar::app::Application application;
        return application.run(argc, argv);
    } catch (const std::exception& exception) {
        SOLAR_LOG_ERROR("Fatal error: {}", exception.what());
        solar::core::Logger::shutdown();
        return 1;
    }
}
