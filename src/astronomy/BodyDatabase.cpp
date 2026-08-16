#include "astronomy/BodyDatabase.hpp"

#include <algorithm>
#include <cctype>

namespace solar::astronomy {
namespace {

constexpr double kDay = 86'400.0;

[[nodiscard]] bool equalIgnoringCase(const std::string_view left, const std::string_view right)
{
    return left.size() == right.size() && std::equal(
        left.begin(), left.end(), right.begin(), [](const char a, const char b) {
            return std::tolower(static_cast<unsigned char>(a)) ==
                   std::tolower(static_cast<unsigned char>(b));
        });
}

} // namespace

BodyDatabase::BodyDatabase()
{
    m_bodies.reserve(10);
    m_bodies.emplace_back("Sun", 10, 10, BodyType::star, 695'700.0, 132'712'440'041.9394,
                          25.38 * kDay, "IAU_SUN", BodyRenderMetadata{{1.0F, 0.78F, 0.25F}, 1.8F});
    m_bodies.emplace_back("Mercury", 199, 199, BodyType::planet, 2'439.4, 22'031.86855,
                          58.646 * kDay, "IAU_MERCURY", BodyRenderMetadata{{0.55F, 0.52F, 0.48F}, 0.8F});
    m_bodies.emplace_back("Venus", 299, 299, BodyType::planet, 6'051.8, 324'858.592,
                          -243.025 * kDay, "IAU_VENUS", BodyRenderMetadata{{0.88F, 0.72F, 0.38F}, 1.0F});
    m_bodies.emplace_back("Earth", 399, 399, BodyType::planet, 6'371.0084, 398'600.435436,
                          86'164.0905, "IAU_EARTH", BodyRenderMetadata{{0.16F, 0.42F, 0.95F}, 1.0F});
    m_bodies.emplace_back("Moon", 301, 301, BodyType::moon, 1'737.4, 4'902.800066,
                          27.321661 * kDay, "IAU_MOON", BodyRenderMetadata{{0.68F, 0.68F, 0.65F}, 0.8F});
    m_bodies.emplace_back("Mars", 499, 4, BodyType::planet, 3'389.5, 42'828.375214,
                          88'642.6848, "IAU_MARS", BodyRenderMetadata{{0.83F, 0.25F, 0.10F}, 0.9F});
    m_bodies.emplace_back("Jupiter", 599, 5, BodyType::planet, 69'911.0, 126'686'534.911,
                          35'729.856, "IAU_JUPITER", BodyRenderMetadata{{0.78F, 0.58F, 0.38F}, 1.25F});
    m_bodies.emplace_back("Saturn", 699, 6, BodyType::planet, 58'232.0, 37'931'207.8,
                          38'362.4, "IAU_SATURN", BodyRenderMetadata{{0.87F, 0.75F, 0.48F}, 1.2F});
    m_bodies.emplace_back("Uranus", 799, 7, BodyType::planet, 25'362.0, 5'793'951.322,
                          -62'064.0, "IAU_URANUS", BodyRenderMetadata{{0.42F, 0.83F, 0.88F}, 1.05F});
    m_bodies.emplace_back("Neptune", 899, 8, BodyType::planet, 24'622.0, 6'835'099.5,
                          57'996.0, "IAU_NEPTUNE", BodyRenderMetadata{{0.18F, 0.30F, 0.92F}, 1.05F});
}

const std::vector<CelestialBody>& BodyDatabase::bodies() const noexcept { return m_bodies; }

const CelestialBody* BodyDatabase::find(const std::string_view bodyName) const noexcept
{
    const auto found = std::find_if(m_bodies.begin(), m_bodies.end(), [&](const CelestialBody& body) {
        return equalIgnoringCase(body.name(), bodyName);
    });
    return found == m_bodies.end() ? nullptr : &*found;
}

const CelestialBody* BodyDatabase::findByNaifId(const int naifId) const noexcept
{
    const auto found = std::find_if(m_bodies.begin(), m_bodies.end(), [&](const CelestialBody& body) {
        return body.naifId() == naifId;
    });
    return found == m_bodies.end() ? nullptr : &*found;
}

} // namespace solar::astronomy
