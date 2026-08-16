#pragma once

#include <string>

#include <glm/vec3.hpp>

namespace solar::astronomy {

enum class BodyType {
    star,
    planet,
    moon,
};

struct BodyRenderMetadata final {
    glm::vec3 color{1.0F};
    float visualRadiusWeight{1.0F};
};

class CelestialBody final {
public:
    CelestialBody(std::string name,
                  int naifId,
                  int ephemerisId,
                  BodyType type,
                  double meanRadiusKm,
                  double gravitationalParameterKm3PerSec2,
                  double rotationPeriodSeconds,
                  std::string bodyFixedFrame,
                  BodyRenderMetadata renderMetadata);

    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] int naifId() const noexcept;
    [[nodiscard]] int ephemerisId() const noexcept;
    [[nodiscard]] BodyType type() const noexcept;
    [[nodiscard]] double meanRadiusKm() const noexcept;
    [[nodiscard]] double gravitationalParameterKm3PerSec2() const noexcept;
    [[nodiscard]] double massKg() const noexcept;
    [[nodiscard]] double rotationPeriodSeconds() const noexcept;
    [[nodiscard]] double surfaceGravityMetersPerSec2() const noexcept;
    [[nodiscard]] double escapeVelocityKmPerSec() const noexcept;
    [[nodiscard]] const std::string& bodyFixedFrame() const noexcept;
    [[nodiscard]] const BodyRenderMetadata& renderMetadata() const noexcept;

private:
    std::string m_name;
    int m_naifId{};
    int m_ephemerisId{};
    BodyType m_type{BodyType::planet};
    double m_meanRadiusKm{};
    double m_gravitationalParameterKm3PerSec2{};
    double m_rotationPeriodSeconds{};
    std::string m_bodyFixedFrame;
    BodyRenderMetadata m_renderMetadata;
};

} // namespace solar::astronomy

