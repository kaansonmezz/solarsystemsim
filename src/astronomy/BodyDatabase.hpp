#pragma once

#include "astronomy/CelestialBody.hpp"

#include <optional>
#include <string_view>
#include <vector>

namespace solar::astronomy {

class BodyDatabase final {
public:
    BodyDatabase();

    [[nodiscard]] const std::vector<CelestialBody>& bodies() const noexcept;
    [[nodiscard]] const CelestialBody* find(std::string_view bodyName) const noexcept;
    [[nodiscard]] const CelestialBody* findByNaifId(int naifId) const noexcept;

private:
    std::vector<CelestialBody> m_bodies;
};

} // namespace solar::astronomy

