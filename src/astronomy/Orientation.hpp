#pragma once

#include "astronomy/ReferenceFrame.hpp"

#include <optional>

#include <glm/mat3x3.hpp>

namespace solar::astronomy {

class CelestialBody;
class SpiceContext;

class Orientation final {
public:
    explicit Orientation(SpiceContext& spiceContext);

    [[nodiscard]] std::optional<glm::dmat3> bodyFixedToReference(
        const CelestialBody& body,
        AxisFrame referenceAxes,
        double ephemerisTime) const;

private:
    SpiceContext& m_spiceContext;
};

} // namespace solar::astronomy

