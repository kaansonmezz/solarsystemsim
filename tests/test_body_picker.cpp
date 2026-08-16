#include "renderer/BodyPicker.hpp"
#include "renderer/Camera.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>

TEST_CASE("Viewport center picks the nearest intersected body", "[renderer][picking]")
{
    solar::renderer::Camera camera;
    camera.focus({0.0F, 0.0F, 0.0F}, 10.0F);
    const glm::vec3 towardCamera = -camera.forward();
    const std::array spheres{
        solar::renderer::PickableSphere{20, glm::vec3{0.0F}, 1.0F},
        solar::renderer::PickableSphere{10, towardCamera * 4.0F, 0.75F},
    };

    const auto picked = solar::renderer::pickNearestSphere(
        camera, 16.0F / 9.0F, {0.5F, 0.5F}, spheres);

    REQUIRE(picked.has_value());
    CHECK(*picked == 10);
}

TEST_CASE("Viewport picking ignores rays that miss bodies", "[renderer][picking]")
{
    solar::renderer::Camera camera;
    camera.focus({0.0F, 0.0F, 0.0F}, 10.0F);
    const std::array spheres{
        solar::renderer::PickableSphere{10, glm::vec3{0.0F}, 0.25F},
    };

    CHECK_FALSE(solar::renderer::pickNearestSphere(
        camera, 1.0F, {0.05F, 0.05F}, spheres).has_value());
}

TEST_CASE("Viewport picking rejects positions outside the image", "[renderer][picking]")
{
    solar::renderer::Camera camera;
    const std::array spheres{
        solar::renderer::PickableSphere{10, glm::vec3{0.0F}, 1.0F},
    };

    CHECK_FALSE(solar::renderer::pickNearestSphere(
        camera, 1.0F, {-0.01F, 0.5F}, spheres).has_value());
}
