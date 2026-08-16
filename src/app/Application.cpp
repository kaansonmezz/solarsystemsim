#include "app/Application.hpp"

#include "core/Constants.hpp"
#include "core/FileSystem.hpp"
#include "core/Logger.hpp"
#include "renderer/BodyPicker.hpp"
#include "renderer/CameraController.hpp"
#include "renderer/Renderer.hpp"
#include "ui/Gui.hpp"
#include "astronomy/BodyDatabase.hpp"
#include "astronomy/Ephemeris.hpp"
#include "astronomy/Orientation.hpp"
#include "astronomy/SpiceContext.hpp"
#include "scene/SolarSystemScene.hpp"
#include "simulation/Spacecraft.hpp"
#include "rocket/EngineDatabase.hpp"
#include "rocket/Vehicle.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <filesystem>
#include <numbers>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace solar::app {
namespace {

std::atomic_bool g_showOpenGlNotifications{false};

void APIENTRY openGlDebugCallback(
    const unsigned int source,
    const unsigned int type,
    const unsigned int id,
    const unsigned int severity,
    const int length,
    const char* message,
    const void* userParam)
{
    static_cast<void>(source);
    static_cast<void>(id);
    static_cast<void>(length);
    static_cast<void>(userParam);
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION && !g_showOpenGlNotifications.load()) {
        return;
    }
    if (type == GL_DEBUG_TYPE_ERROR || severity == GL_DEBUG_SEVERITY_HIGH) {
        SOLAR_LOG_ERROR("OpenGL: {}", message);
    } else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
        SOLAR_LOG_WARN("OpenGL: {}", message);
    } else {
        SOLAR_LOG_INFO("OpenGL: {}", message);
    }
}

[[nodiscard]] bool keyDown(GLFWwindow* window, const int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

[[nodiscard]] rocket::Vehicle makeDefaultRocketVehicle(
    const rocket::EngineDatabase& engineDatabase)
{
    rocket::Vehicle vehicle;
    vehicle.name = "Two Stage Sample Vehicle";
    vehicle.payloadMassKg = 5'000.0;

    rocket::Stage first;
    first.name = "Stage 1 — First Burn";
    first.structuralMassKg = 20'000.0;
    first.propellant.massKg = 100'000.0;
    first.engineCount = 2;

    rocket::Stage second;
    second.name = "Stage 2 — Second Burn";
    second.structuralMassKg = 5'000.0;
    second.propellant.massKg = 20'000.0;
    second.engineCount = 1;

    if (!engineDatabase.engines().empty()) {
        first.engine = engineDatabase.engines().front();
        second.engine = engineDatabase.engines().size() > 1
            ? engineDatabase.engines()[1] : engineDatabase.engines().front();
    } else {
        first.engine = {"Custom Sample Engine", "", "User-editable fallback",
                        500'000.0, 450'000.0, 320.0, 290.0, 500.0, 0.0, 1.0};
        second.engine = first.engine;
    }
    vehicle.stages.push_back(std::move(first));
    vehicle.stages.push_back(std::move(second));
    return vehicle;
}

} // namespace

Application::Application() = default;
Application::~Application() { shutdown(); }

int Application::run(const int argc, char** argv)
{
    initialize(argc, argv);
    m_previousFrameTime = std::chrono::steady_clock::now();

    while (glfwWindowShouldClose(m_window) == GLFW_FALSE && !m_state.requestExit) {
        glfwPollEvents();
        const auto now = std::chrono::steady_clock::now();
        const double realDeltaSeconds = std::chrono::duration<double>(now - m_previousFrameTime).count();
        m_previousFrameTime = now;
        const double boundedDelta = std::clamp(realDeltaSeconds, 0.0, 0.25);

        m_simulationClock.advance(boundedDelta);
        m_solarSystemScene->update(m_simulationClock.simulationTime());
        if (m_spacecraft) m_spacecraft->update(m_simulationClock.simulationTime());
        updateFrameStatistics(boundedDelta);

        m_gui->beginFrame();
        processInput(static_cast<float>(boundedDelta));
        if (m_solarSystemScene->followSelected) {
            if (const auto* selected = m_solarSystemScene->selectedObject()) {
                m_renderer->camera().followTarget(selected->renderPosition);
            }
        }
        if (m_state.followSpacecraft && m_spacecraft) {
            glm::vec3 center{};
            for (const auto& object : m_solarSystemScene->objects()) {
                if (object.body->ephemerisId() == m_spacecraft->centralBodyEphemerisId()) {
                    center = object.renderPosition;
                    break;
                }
            }
            m_renderer->camera().followTarget(center +
                m_solarSystemScene->renderTransform().toRenderPosition(
                    m_spacecraft->current().relativeState.positionKm));
        }
        g_showOpenGlNotifications.store(m_state.showOpenGlNotifications);

        m_renderer->renderScene(
            m_state.viewport.width, m_state.viewport.height, *m_solarSystemScene,
            m_spacecraft.get());
        m_gui->renderPanels(
            m_state, *m_renderer, m_simulationClock, *m_solarSystemScene, *m_spiceContext,
            *m_bodyDatabase, m_spacecraft, *m_rocketVehicle, *m_engineDatabase);

        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, framebufferWidth, framebufferHeight);
        glClearColor(0.025F, 0.029F, 0.039F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        m_gui->endFrame();
        glfwSwapBuffers(m_window);
    }

    return 0;
}

void Application::initialize(const int argc, char** argv)
{
    core::Logger::initialize();
    const std::filesystem::path executablePath = argc > 0 && argv != nullptr
        ? std::filesystem::path(argv[0]) : std::filesystem::path{};
    core::FileSystem::initialize(executablePath);
    loadConfig();

    m_spiceContext = std::make_unique<astronomy::SpiceContext>();
    const std::filesystem::path configuredKernelPath{m_config.kernelDirectory};
    const auto kernelDirectory = configuredKernelPath.is_absolute()
        ? configuredKernelPath
        : core::FileSystem::resourceRoot() / configuredKernelPath;
    if (!m_spiceContext->loadKernelManifest(kernelDirectory)) {
        SOLAR_LOG_ERROR("Astronomy initialization incomplete: {}", m_spiceContext->lastError());
    }
    m_bodyDatabase = std::make_unique<astronomy::BodyDatabase>();
    auto engineResult = rocket::loadEngineDatabase(core::FileSystem::config("engines.json"));
    for (const auto& error : engineResult.errors) SOLAR_LOG_WARN("{}", error);
    m_engineDatabase = std::make_unique<rocket::EngineDatabase>(std::move(engineResult.database));
    m_rocketVehicle = std::make_unique<rocket::Vehicle>(
        makeDefaultRocketVehicle(*m_engineDatabase));
    m_ephemeris = std::make_unique<astronomy::Ephemeris>(*m_spiceContext);
    m_orientation = std::make_unique<astronomy::Orientation>(*m_spiceContext);
    m_solarSystemScene = std::make_unique<scene::SolarSystemScene>(
        *m_bodyDatabase, *m_ephemeris, *m_orientation, *m_spiceContext);
    m_solarSystemScene->setReferenceOrigin(m_config.defaultOrigin);
    m_solarSystemScene->setAxisFrame(m_config.defaultFrame);
    m_solarSystemScene->setDistanceScale(m_config.distanceScale);
    m_solarSystemScene->setBodyRadiusScale(m_config.bodyRadiusScale);
    m_solarSystemScene->showOrbits = m_config.showOrbits;
    m_solarSystemScene->showLabels = m_config.showLabels;
    m_solarSystemScene->update(m_simulationClock.simulationTime());
    if (const auto* earth = m_bodyDatabase->find("Earth")) {
        auto initialSpacecraft = simulation::Spacecraft::fromApsisAltitudes(
            "Earth Test Vehicle", earth->name(), earth->ephemerisId(), earth->meanRadiusKm(),
            earth->gravitationalParameterKm3PerSec2(), 200.0, 200.0,
            28.5 * std::numbers::pi / 180.0, 0.0, 0.0, 0.0,
            m_simulationClock.simulationTime());
        if (initialSpacecraft) {
            m_spacecraft = std::make_unique<simulation::Spacecraft>(std::move(*initialSpacecraft));
            SOLAR_LOG_INFO("Created default 200 km Earth test orbit");
        }
    }

    glfwSetErrorCallback(glfwErrorCallback);
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("GLFW initialization failed");
    }
    m_glfwInitialized = true;
    SOLAR_LOG_INFO("GLFW initialized");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, core::kRequiredOpenGlMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, core::kRequiredOpenGlMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    m_window = glfwCreateWindow(
        m_config.windowWidth, m_config.windowHeight, "SOLAR", nullptr, nullptr);
    if (m_window == nullptr) {
        throw std::runtime_error("GLFW window creation failed; OpenGL 4.6 may be unavailable");
    }
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(m_config.verticalSync ? 1 : 0);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetScrollCallback(m_window, scrollCallback);

    const int gladVersion = gladLoadGL(
        reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));
    if (gladVersion == 0) {
        throw std::runtime_error("GLAD failed to load OpenGL functions");
    }
    if (GLAD_GL_VERSION_4_6 == 0) {
        throw std::runtime_error("SOLAR requires an OpenGL 4.6 context");
    }

    SOLAR_LOG_INFO("OpenGL {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    SOLAR_LOG_INFO("GPU: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

#ifndef NDEBUG
    if (GLAD_GL_KHR_debug != 0 || GLAD_GL_VERSION_4_3 != 0) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openGlDebugCallback, nullptr);
        SOLAR_LOG_INFO("OpenGL debug output enabled");
    }
#endif

    m_renderer = std::make_unique<renderer::Renderer>();
    m_renderer->camera().setFieldOfView(m_config.fieldOfView);
    m_renderer->camera().setOrbitSensitivity(m_config.orbitSensitivity);
    m_renderer->camera().setClipPlanes(0.001F, 10'000.0F);
    m_renderer->initialize();
    m_renderer->camera().focus(glm::vec3{0.0F}, 340.0F);
    m_cameraController = std::make_unique<renderer::CameraController>(m_renderer->camera());
    m_gui = std::make_unique<ui::Gui>();
    m_gui->initialize(m_window);
    m_initialized = true;
    SOLAR_LOG_INFO("Application initialized; resources: {}", core::FileSystem::resourceRoot().string());
}

void Application::shutdown() noexcept
{
    if (m_initialized) {
        SOLAR_LOG_INFO("Application shutting down");
    }
    if (m_gui) {
        m_gui->shutdown();
        m_gui.reset();
    }
    m_cameraController.reset();
    m_renderer.reset();
    m_solarSystemScene.reset();
    m_spacecraft.reset();
    m_rocketVehicle.reset();
    m_engineDatabase.reset();
    m_orientation.reset();
    m_ephemeris.reset();
    m_bodyDatabase.reset();
    m_spiceContext.reset();
    if (m_window != nullptr) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    if (m_glfwInitialized) {
        glfwTerminate();
        m_glfwInitialized = false;
    }
    m_initialized = false;
    core::Logger::shutdown();
}

void Application::loadConfig()
{
    const auto path = core::FileSystem::config("app.json");
    auto result = loadAppConfig(path);
    m_config = std::move(result.config);
    for (const auto& warning : result.warnings) SOLAR_LOG_WARN("{}", warning);
    if (result.warnings.empty()) SOLAR_LOG_INFO("Configuration loaded from {}", path.string());
}

void Application::processInput(const float realDeltaSeconds)
{
    ImGuiIO& io = ImGui::GetIO();
    if (std::exchange(m_state.viewport.leftClickPending, false)) {
        focusClickedBody();
    }
    if (!io.WantTextInput && !ImGui::IsAnyItemActive()) {
        if (ImGui::IsKeyPressed(ImGuiKey_Space, false)) m_simulationClock.togglePaused();
        if (ImGui::IsKeyPressed(ImGuiKey_R, false)) m_simulationClock.returnToRealTime();
        if (ImGui::IsKeyPressed(ImGuiKey_I, false)) {
            const auto* selected = m_solarSystemScene->selectedObject();
            if (selected != nullptr && selected->orbitalRenderData) {
                const auto& geometry = *selected->orbitalRenderData;
                m_solarSystemScene->showOrbits = true;
                m_solarSystemScene->showReferencePlane = true;
                m_solarSystemScene->showOrbitalPlane = true;
                m_solarSystemScene->followSelected = false;
                m_renderer->camera().focusFromDirection(
                    geometry.centralBodyPosition, geometry.orbitalBasisU,
                    std::max(geometry.planeRadius * 2.35F, 0.8F));
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) m_state.requestExit = true;
        constexpr int shortcutIds[]{199, 299, 399, 499, 599, 699, 799, 899, 10};
        constexpr ImGuiKey shortcutKeys[]{ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, ImGuiKey_4,
                                           ImGuiKey_5, ImGuiKey_6, ImGuiKey_7, ImGuiKey_8, ImGuiKey_9};
        for (std::size_t index = 0; index < std::size(shortcutIds); ++index) {
            if (ImGui::IsKeyPressed(shortcutKeys[index], false)) {
                m_solarSystemScene->selectBody(shortcutIds[index]);
            }
        }
    }

    double mouseX = 0.0;
    double mouseY = 0.0;
    glfwGetCursorPos(m_window, &mouseX, &mouseY);

    // ImGui owns the viewport image too, so WantCaptureMouse cannot distinguish it
    // from controls in other panels. The viewport item hover state is the boundary.
    const bool canUseMouse = m_state.viewport.hovered && !ImGui::IsAnyItemActive();
    m_cameraController->update({
        mouseX,
        mouseY,
        static_cast<float>(m_scrollDelta),
        canUseMouse,
        glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS,
        glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS,
    });
    m_scrollDelta = 0.0;

    if (!m_state.viewport.focused || io.WantTextInput || ImGui::IsAnyItemActive()) {
        return;
    }
    glm::vec3 movement{0.0F};
    if (keyDown(m_window, GLFW_KEY_A)) movement.x -= 1.0F;
    if (keyDown(m_window, GLFW_KEY_D)) movement.x += 1.0F;
    if (keyDown(m_window, GLFW_KEY_Q)) movement.y -= 1.0F;
    if (keyDown(m_window, GLFW_KEY_E)) movement.y += 1.0F;
    if (keyDown(m_window, GLFW_KEY_S)) movement.z -= 1.0F;
    if (keyDown(m_window, GLFW_KEY_W)) movement.z += 1.0F;
    m_renderer->camera().moveLocal(movement, realDeltaSeconds);
}

void Application::focusClickedBody()
{
    std::vector<renderer::PickableSphere> pickableBodies;
    pickableBodies.reserve(m_solarSystemScene->objects().size());
    for (const auto& object : m_solarSystemScene->objects()) {
        if (!object.visible || !object.stateValid) continue;
        pickableBodies.push_back({
            object.body->naifId(), object.renderPosition, object.renderRadius});
    }

    const float aspectRatio = static_cast<float>(m_state.viewport.width) /
                              static_cast<float>(std::max(m_state.viewport.height, 1));
    const auto bodyId = renderer::pickNearestSphere(
        m_renderer->camera(), aspectRatio,
        {m_state.viewport.clickNormalizedX, m_state.viewport.clickNormalizedY},
        pickableBodies);
    if (!bodyId || !m_solarSystemScene->selectBody(*bodyId)) return;

    m_solarSystemScene->setReferenceOrigin(astronomy::OriginMode::selectedBody);
    m_solarSystemScene->followSelected = true;
    m_state.followSpacecraft = false;
    const auto* selected = m_solarSystemScene->selectedObject();
    if (selected != nullptr) {
        m_renderer->camera().focus(
            selected->renderPosition, std::max(selected->renderRadius * 8.0F, 0.8F));
    }
}

void Application::updateFrameStatistics(const double realDeltaSeconds)
{
    if (realDeltaSeconds <= 0.0) {
        return;
    }
    const double instantaneousFps = 1.0 / realDeltaSeconds;
    constexpr double smoothing = 0.08;
    if (m_state.framesPerSecond == 0.0) {
        m_state.framesPerSecond = instantaneousFps;
    } else {
        m_state.framesPerSecond += (instantaneousFps - m_state.framesPerSecond) * smoothing;
    }
    m_state.frameTimeMilliseconds = 1'000.0 / m_state.framesPerSecond;
}

void Application::glfwErrorCallback(const int error, const char* description)
{
    SOLAR_LOG_ERROR("GLFW error {}: {}", error, description != nullptr ? description : "unknown");
}

void Application::scrollCallback(GLFWwindow* window, const double xOffset, const double yOffset)
{
    static_cast<void>(xOffset);
    auto* application = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (application != nullptr) {
        application->m_scrollDelta += yOffset;
    }
}

} // namespace solar::app
