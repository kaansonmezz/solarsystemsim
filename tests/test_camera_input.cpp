#include "renderer/Camera.hpp"
#include "renderer/CameraController.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/geometric.hpp>

namespace {

using solar::renderer::CameraInputState;

[[nodiscard]] CameraInputState input(
    const double x,
    const double y,
    const bool allowed,
    const bool middle,
    const bool right = false,
    const float scroll = 0.0F)
{
    return {x, y, scroll, allowed, middle, right};
}

} // namespace

TEST_CASE("Middle mouse drag orbits without zooming", "[camera][input]")
{
    solar::renderer::Camera camera;
    solar::renderer::CameraController controller{camera};
    camera.focus({2.0F, 3.0F, -4.0F}, 30.0F);
    const float distance = camera.distance();
    const float yaw = camera.yaw();
    const float pitch = camera.pitch();

    controller.update(input(400.0, 300.0, true, true));
    CHECK(camera.yaw() == yaw);
    CHECK(camera.pitch() == pitch);

    // Scroll is deliberately included to model drivers that synthesize it while
    // the middle button is held. MMB orbit must still preserve distance.
    controller.update(input(500.0, 350.0, true, true, false, 4.0F));
    CHECK(camera.yaw() != yaw);
    CHECK(camera.pitch() != pitch);
    CHECK(camera.distance() == Catch::Approx(distance).margin(1.0e-5F));
    CHECK(glm::distance(camera.position(), camera.target()) ==
          Catch::Approx(distance).margin(1.0e-4F));
}

TEST_CASE("Wheel scroll zooms without rotating", "[camera][input]")
{
    solar::renderer::Camera camera;
    solar::renderer::CameraController controller{camera};
    const float distance = camera.distance();
    const float yaw = camera.yaw();
    const float pitch = camera.pitch();

    controller.update(input(400.0, 300.0, true, false, false, 1.0F));

    CHECK(camera.distance() < distance);
    CHECK(camera.yaw() == yaw);
    CHECK(camera.pitch() == pitch);
}

TEST_CASE("Middle drag must begin inside viewport", "[camera][input]")
{
    solar::renderer::Camera camera;
    solar::renderer::CameraController controller{camera};
    const glm::vec3 position = camera.position();

    controller.update(input(100.0, 100.0, false, true));
    controller.update(input(300.0, 250.0, true, true));
    CHECK(camera.position() == position);

    controller.update(input(300.0, 250.0, true, false));
    controller.update(input(300.0, 250.0, true, true));
    CHECK(camera.position() == position);
    controller.update(input(340.0, 280.0, true, true));
    CHECK(camera.position() != position);
}

TEST_CASE("Right mouse drag pans without changing orbit distance", "[camera][input]")
{
    solar::renderer::Camera camera;
    solar::renderer::CameraController controller{camera};
    const glm::vec3 target = camera.target();
    const float distance = camera.distance();

    controller.update(input(200.0, 200.0, true, false, true));
    controller.update(input(250.0, 230.0, true, false, true));

    CHECK(camera.target() != target);
    CHECK(camera.distance() == Catch::Approx(distance));
    CHECK(glm::distance(camera.position(), camera.target()) ==
          Catch::Approx(distance).margin(1.0e-4F));
}
