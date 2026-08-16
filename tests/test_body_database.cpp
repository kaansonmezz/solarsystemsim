#include "astronomy/BodyDatabase.hpp"

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <string>

TEST_CASE("BodyDatabase contains valid unique required bodies", "[astronomy][database]")
{
    const solar::astronomy::BodyDatabase database;
    const std::set<std::string> required{
        "Sun", "Mercury", "Venus", "Earth", "Moon", "Mars",
        "Jupiter", "Saturn", "Uranus", "Neptune",
    };
    std::set<std::string> names;
    std::set<int> ids;

    CHECK(database.bodies().size() == required.size());
    for (const auto& body : database.bodies()) {
        CHECK(required.contains(body.name()));
        CHECK(names.insert(body.name()).second);
        CHECK(ids.insert(body.naifId()).second);
        CHECK(body.meanRadiusKm() > 0.0);
        CHECK(body.gravitationalParameterKm3PerSec2() > 0.0);
        CHECK(body.massKg() > 0.0);
        CHECK(body.renderMetadata().visualRadiusWeight > 0.0F);
    }
    CHECK(database.find("earth") != nullptr);
    CHECK(database.findByNaifId(499) != nullptr);
    CHECK(database.find("Ceres") == nullptr);
}

