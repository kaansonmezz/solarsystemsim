#include "renderer/Camera.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/geometric.hpp>

#include <cmath>

namespace {

[[nodiscard]] bool finite(const glm::vec3& value)
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

[[nodiscard]] bool finite(const glm::mat4& value)
{
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            if (!std::isfinite(value[column][row])) return false;
        }
    }
    return true;
}

} // namespace

TEST_CASE("Camera orbit preserves target distance", "[camera]")
{
    solar::renderer::Camera camera;
    camera.focus({4.0F, -2.0F, 7.0F}, 25.0F);
    const float before = glm::distance(camera.position(), camera.target());

    camera.orbit(180.0F, -95.0F);

    CHECK(glm::distance(camera.position(), camera.target()) ==
          Catch::Approx(before).margin(1.0e-4F));
    CHECK(camera.distance() == Catch::Approx(before).margin(1.0e-5F));
}

TEST_CASE("Camera wheel zoom changes distance without changing orbit angles", "[camera]")
{
    solar::renderer::Camera camera;
    const float distance = camera.distance();
    const float yaw = camera.yaw();
    const float pitch = camera.pitch();

    camera.zoom(1.0F);

    CHECK(camera.distance() < distance);
    CHECK(camera.yaw() == yaw);
    CHECK(camera.pitch() == pitch);
}

TEST_CASE("Camera orbit clamps pitch and keeps a finite basis", "[camera]")
{
    solar::renderer::Camera camera;
    camera.orbit(0.0F, -100'000.0F);
    CHECK(camera.pitch() == Catch::Approx(89.0F));
    CHECK(finite(camera.forward()));
    CHECK(finite(camera.right()));
    CHECK(finite(camera.up()));
    CHECK(finite(camera.getViewMatrix()));

    camera.orbit(0.0F, 100'000.0F);
    CHECK(camera.pitch() == Catch::Approx(-89.0F));
    CHECK(finite(camera.getViewMatrix()));
}

TEST_CASE("Camera yaw remains normalized after large drags", "[camera]")
{
    solar::renderer::Camera camera;
    camera.setOrbitSensitivity(2.0F);
    camera.orbit(1'000'000.0F, 0.0F);

    CHECK(std::isfinite(camera.yaw()));
    CHECK(std::abs(camera.yaw()) <= 180.0F);
}

TEST_CASE("Camera target changes preserve valid orbit state", "[camera]")
{
    solar::renderer::Camera camera;
    camera.orbit(90.0F, 40.0F);
    const glm::vec3 originalDirection = camera.forward();
    camera.followTarget({120.0F, -45.0F, 8.0F});

    CHECK(glm::distance(camera.position(), camera.target()) ==
          Catch::Approx(camera.distance()).margin(1.0e-4F));
    CHECK(glm::dot(camera.forward(), originalDirection) ==
          Catch::Approx(1.0F).margin(1.0e-5F));
    CHECK(finite(camera.position()));
    CHECK(finite(camera.target()));
    CHECK(finite(camera.getViewMatrix()));
}

TEST_CASE("Camera can frame an orbital plane along a prepared node direction", "[camera]")
{
    solar::renderer::Camera camera;
    const glm::vec3 target{3.0F, -2.0F, 5.0F};
    const glm::vec3 nodeDirection{0.6F, 0.0F, -0.8F};
    camera.focusFromDirection(target, nodeDirection, 42.0F);

    CHECK(camera.target() == target);
    CHECK(camera.distance() == Catch::Approx(42.0F));
    CHECK(glm::distance(camera.position(), target) == Catch::Approx(42.0F));
    CHECK(glm::dot(glm::normalize(camera.position() - target), nodeDirection) ==
          Catch::Approx(1.0F).margin(1.0e-6F));
    CHECK(finite(camera.getViewMatrix()));
}
