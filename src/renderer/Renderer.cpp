#include "renderer/Renderer.hpp"

#include "core/FileSystem.hpp"
#include "core/Logger.hpp"
#include "renderer/Framebuffer.hpp"
#include "renderer/GridRenderer.hpp"
#include "renderer/OrbitRenderer.hpp"
#include "renderer/OrbitalGeometryRenderer.hpp"
#include "renderer/PlanetRenderer.hpp"
#include "renderer/PrimitiveRenderer.hpp"
#include "renderer/Shader.hpp"
#include "renderer/SpacecraftRenderer.hpp"
#include "renderer/TrajectoryRenderer.hpp"
#include "scene/SolarSystemScene.hpp"
#include "astronomy/CelestialBody.hpp"
#include "simulation/Spacecraft.hpp"

#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <algorithm>
#include <iomanip>
#include <numbers>
#include <sstream>

namespace solar::renderer {
namespace {

[[nodiscard]] glm::mat4 renderOrientation(const glm::dmat3& bodyFixedToReference)
{
    const glm::mat3 coordinateMap{
        {1.0F, 0.0F, 0.0F},
        {0.0F, 0.0F, -1.0F},
        {0.0F, 1.0F, 0.0F},
    };
    return glm::mat4{coordinateMap * glm::mat3{bodyFixedToReference}};
}

} // namespace

Renderer::Renderer() = default;
Renderer::~Renderer() = default;

void Renderer::initialize()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_MULTISAMPLE);

    m_framebuffer = std::make_unique<Framebuffer>(1280, 720);
    m_planetShader = std::make_unique<Shader>(
        core::FileSystem::shader("planet.vert"), core::FileSystem::shader("planet.frag"));
    m_ringShader = std::make_unique<Shader>(
        core::FileSystem::shader("planet.vert"), core::FileSystem::shader("ring.frag"));
    m_lineShader = std::make_unique<Shader>(
        core::FileSystem::shader("line.vert"), core::FileSystem::shader("line.frag"));
    m_gridRenderer = std::make_unique<GridRenderer>(40, 10.0F);
    m_primitiveRenderer = std::make_unique<PrimitiveRenderer>();
    m_planetRenderer = std::make_unique<PlanetRenderer>();
    m_orbitRenderer = std::make_unique<OrbitRenderer>();
    m_orbitalGeometryRenderer = std::make_unique<OrbitalGeometryRenderer>();
    m_trajectoryRenderer = std::make_unique<TrajectoryRenderer>();
    m_spacecraftRenderer = std::make_unique<SpacecraftRenderer>();
    SOLAR_LOG_INFO("Renderer initialized");
}

void Renderer::renderScene(
    const int width, const int height, const scene::SolarSystemScene& scene,
    const simulation::Spacecraft* spacecraft)
{
    resize(width, height);
    m_framebuffer->bind();
    glViewport(0, 0, m_framebuffer->width(), m_framebuffer->height());
    glClearColor(0.018F, 0.024F, 0.038F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    const float aspect = static_cast<float>(m_framebuffer->width()) /
                         static_cast<float>(m_framebuffer->height());
    const glm::mat4 view = m_camera.getViewMatrix();
    const glm::mat4 projection = m_camera.getProjectionMatrix(aspect);
    const glm::mat4 identity{1.0F};

    m_lineShader->bind();
    m_lineShader->set("uView", view);
    m_lineShader->set("uProjection", projection);
    m_lineShader->set("uModel", identity);
    m_lineShader->set("uAlpha", 1.0F);
    m_lineShader->set("uColorScale", glm::vec3{1.0F});
    if (scene.showGrid) m_gridRenderer->render(*m_lineShader);
    if (scene.showAxes) m_primitiveRenderer->renderAxes(*m_lineShader);

    m_orbitRenderer->synchronize(scene);
    if (scene.showOrbits) m_orbitRenderer->render(*m_lineShader, scene);
    const auto* selectedObject = scene.selectedObject();
    if (selectedObject != nullptr && selectedObject->orbitalRenderData) {
        m_orbitalGeometryRenderer->render(
            *m_lineShader, *selectedObject->orbitalRenderData,
            scene.showReferencePlane, scene.showOrbitalPlane,
            scene.showOrbitalNormal, scene.showOrbitNodes);
    }
    if (spacecraft != nullptr) {
        m_trajectoryRenderer->synchronize(spacecraft->visualizationData(), scene);
        m_trajectoryRenderer->render(*m_lineShader, spacecraft->visualizationData(), scene);
    }

    glm::vec3 sunPosition{0.0F};
    for (const auto& object : scene.objects()) {
        if (object.body->naifId() == 10) sunPosition = object.renderPosition;
    }

    m_planetShader->bind();
    m_planetShader->set("uView", view);
    m_planetShader->set("uProjection", projection);
    m_planetShader->set("uSunPosition", sunPosition);

    m_ringShader->bind();
    m_ringShader->set("uView", view);
    m_ringShader->set("uProjection", projection);
    m_ringShader->set("uSunPosition", sunPosition);
    m_ringShader->set("uEmissive", false);

    m_screenLabels.clear();
    const auto appendScreenLabel = [&](std::string text,
                                       const glm::vec3& position,
                                       const glm::vec3& color) {
        const glm::vec4 clip = projection * view * glm::vec4{position, 1.0F};
        if (clip.w <= 0.0F) return;
        const glm::vec3 ndc = glm::vec3{clip} / clip.w;
        if (ndc.z < -1.0F || ndc.z > 1.0F ||
            ndc.x < -1.1F || ndc.x > 1.1F || ndc.y < -1.1F || ndc.y > 1.1F) return;
        m_screenLabels.push_back({
            std::move(text),
            {ndc.x * 0.5F + 0.5F, 1.0F - (ndc.y * 0.5F + 0.5F)},
            color,
        });
    };

    if (selectedObject != nullptr && selectedObject->orbitalRenderData &&
        selectedObject->osculatingOrbit) {
        const auto& renderData = *selectedObject->orbitalRenderData;
        if (scene.showReferencePlane) {
            appendScreenLabel(
                "Reference Plane: " + std::string(astronomy::name(scene.referenceFrame().axes)),
                renderData.centralBodyPosition + renderData.referenceBasisU *
                    renderData.planeRadius * 0.93F,
                {0.35F, 0.68F, 1.0F});
        }
        if (scene.showOrbitalPlane) {
            std::ostringstream label;
            label << selectedObject->body->name() << " Orbital Plane   i = " << std::fixed
                  << std::setprecision(3)
                  << selectedObject->osculatingOrbit->elements.inclinationRad * 180.0 /
                     std::numbers::pi
                  << " deg";
            appendScreenLabel(
                label.str(),
                renderData.centralBodyPosition + renderData.orbitalBasisU *
                    renderData.planeRadius * 0.72F + renderData.orbitalNormal *
                    renderData.planeRadius * 0.035F,
                {1.0F, 0.58F, 0.22F});
        }
        if (scene.showOrbitalNormal) {
            appendScreenLabel(
                "+h", renderData.centralBodyPosition + renderData.orbitalNormal *
                    renderData.planeRadius * 0.42F,
                {0.9F, 0.55F, 1.0F});
        }
        if (scene.showOrbitNodes) {
            if (renderData.ascendingNodePosition) {
                appendScreenLabel("AN", *renderData.ascendingNodePosition,
                                  {1.0F, 0.95F, 0.25F});
            }
            if (renderData.descendingNodePosition) {
                appendScreenLabel("DN", *renderData.descendingNodePosition,
                                  {1.0F, 0.95F, 0.25F});
            }
        }
    }
    for (const auto& object : scene.objects()) {
        if (!object.visible || !object.stateValid) continue;
        const glm::mat4 orientation = renderOrientation(object.bodyFixedToReference);
        const glm::mat4 model = glm::translate(identity, object.renderPosition) * orientation *
                                glm::scale(identity, glm::vec3{object.renderRadius});
        m_planetShader->bind();
        m_planetShader->set("uEmissive", object.body->type() == astronomy::BodyType::star);
        m_planetRenderer->renderBody(
            *m_planetShader, model, object.body->renderMetadata().color);

        if (object.body->name() == "Saturn") {
            m_planetRenderer->renderRing(
                *m_ringShader, model, object.body->renderMetadata().color * 0.9F);
        }
        if (scene.showRotationAxis && object.body->naifId() == scene.selectedBodyId()) {
            m_lineShader->bind();
            float axisRadius = object.renderRadius;
            if (object.orbitalRenderData) {
                axisRadius = std::max(axisRadius, object.orbitalRenderData->planeRadius * 0.04F);
            }
            const glm::mat4 axisModel = glm::translate(identity, object.renderPosition) *
                orientation * glm::scale(identity, glm::vec3{axisRadius});
            m_planetRenderer->renderRotationAxis(*m_lineShader, axisModel);
        }
        if (scene.showEquatorialPlane && object.body->naifId() == scene.selectedBodyId()) {
            float equatorialRadius = object.renderRadius * 1.8F;
            if (object.orbitalRenderData) {
                equatorialRadius = std::max(
                    equatorialRadius, object.orbitalRenderData->planeRadius * 0.025F);
            }
            const glm::mat4 planeModel = glm::translate(identity, object.renderPosition) * orientation *
                                         glm::scale(identity, glm::vec3{equatorialRadius});
            m_planetRenderer->renderRing(*m_ringShader, planeModel, {0.3F, 0.65F, 1.0F});
        }

        if (scene.showLabels) {
            appendScreenLabel(
                object.body->name(), object.renderPosition, object.body->renderMetadata().color);
        }
    }

    if (spacecraft != nullptr) {
        m_planetShader->bind();
        m_planetShader->set("uView", view);
        m_planetShader->set("uProjection", projection);
        m_spacecraftRenderer->render(*m_planetShader, *spacecraft, scene);

        glm::vec3 center{};
        for (const auto& object : scene.objects()) {
            if (object.body->ephemerisId() == spacecraft->centralBodyEphemerisId()) {
                center = object.renderPosition;
                break;
            }
        }
        const glm::vec3 spacecraftPosition = center + scene.renderTransform().toRenderPosition(
            spacecraft->visualizationData().spacecraftPositionKm);
        const glm::vec4 clip = projection * view * glm::vec4{spacecraftPosition, 1.0F};
        if (clip.w > 0.0F) {
            const glm::vec3 ndc = glm::vec3{clip} / clip.w;
            m_screenLabels.push_back({spacecraft->name(),
                {ndc.x * 0.5F + 0.5F, 1.0F - (ndc.y * 0.5F + 0.5F)},
                {0.25F, 1.0F, 0.9F}});
        }
        for (const auto& marker : m_trajectoryRenderer->markerPositions()) {
            const glm::vec3 markerPosition = center + marker.position;
            const glm::vec4 markerClip = projection * view * glm::vec4{markerPosition, 1.0F};
            if (markerClip.w <= 0.0F) continue;
            const glm::vec3 ndc = glm::vec3{markerClip} / markerClip.w;
            m_screenLabels.push_back({marker.label,
                {ndc.x * 0.5F + 0.5F, 1.0F - (ndc.y * 0.5F + 0.5F)},
                {1.0F, 1.0F, 0.35F}});
        }
    }

    Framebuffer::unbind();
}

void Renderer::resize(const int width, const int height)
{
    if (m_framebuffer) {
        m_framebuffer->resize(std::max(width, 1), std::max(height, 1));
    }
}

unsigned int Renderer::viewportTexture() const noexcept
{
    return m_framebuffer ? m_framebuffer->colorTexture() : 0;
}

Camera& Renderer::camera() noexcept { return m_camera; }
const Camera& Renderer::camera() const noexcept { return m_camera; }
int Renderer::viewportWidth() const noexcept { return m_framebuffer ? m_framebuffer->width() : 0; }
int Renderer::viewportHeight() const noexcept { return m_framebuffer ? m_framebuffer->height() : 0; }
const std::vector<ScreenLabel>& Renderer::screenLabels() const noexcept { return m_screenLabels; }

} // namespace solar::renderer
