#include "astronomy/CelestialBody.hpp"

#include <cmath>
#include <utility>

namespace solar::astronomy {
namespace {

constexpr double kGravitationalConstantKm3PerKgSec2 = 6.67430e-20;

} // namespace

CelestialBody::CelestialBody(
    std::string name,
    const int naifId,
    const int ephemerisId,
    const BodyType type,
    const double meanRadiusKm,
    const double gravitationalParameterKm3PerSec2,
    const double rotationPeriodSeconds,
    std::string bodyFixedFrame,
    const BodyRenderMetadata renderMetadata)
    : m_name(std::move(name)),
      m_naifId(naifId),
      m_ephemerisId(ephemerisId),
      m_type(type),
      m_meanRadiusKm(meanRadiusKm),
      m_gravitationalParameterKm3PerSec2(gravitationalParameterKm3PerSec2),
      m_rotationPeriodSeconds(rotationPeriodSeconds),
      m_bodyFixedFrame(std::move(bodyFixedFrame)),
      m_renderMetadata(renderMetadata)
{
}

const std::string& CelestialBody::name() const noexcept { return m_name; }
int CelestialBody::naifId() const noexcept { return m_naifId; }
int CelestialBody::ephemerisId() const noexcept { return m_ephemerisId; }
BodyType CelestialBody::type() const noexcept { return m_type; }
double CelestialBody::meanRadiusKm() const noexcept { return m_meanRadiusKm; }
double CelestialBody::gravitationalParameterKm3PerSec2() const noexcept { return m_gravitationalParameterKm3PerSec2; }
double CelestialBody::massKg() const noexcept { return m_gravitationalParameterKm3PerSec2 / kGravitationalConstantKm3PerKgSec2; }
double CelestialBody::rotationPeriodSeconds() const noexcept { return m_rotationPeriodSeconds; }
const std::string& CelestialBody::bodyFixedFrame() const noexcept { return m_bodyFixedFrame; }
const BodyRenderMetadata& CelestialBody::renderMetadata() const noexcept { return m_renderMetadata; }

double CelestialBody::surfaceGravityMetersPerSec2() const noexcept
{
    return m_gravitationalParameterKm3PerSec2 /
           (m_meanRadiusKm * m_meanRadiusKm) * 1'000.0;
}

double CelestialBody::escapeVelocityKmPerSec() const noexcept
{
    return std::sqrt(2.0 * m_gravitationalParameterKm3PerSec2 / m_meanRadiusKm);
}

} // namespace solar::astronomy

