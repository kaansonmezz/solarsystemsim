cmake_minimum_required(VERSION 3.24)

project(SOLAR VERSION 0.4.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Dependencies)
include(CSpice)
include(CompilerWarnings)
include(Sanitizers)

option(SOLAR_BUILD_TESTS "Build SOLAR tests" ON)
option(SOLAR_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

solar_setup_dependencies()
solar_setup_cspice()

add_library(solar_core
    src/core/FileSystem.cpp
    src/core/Logger.cpp
    src/core/Time.cpp
    src/simulation/SimulationClock.cpp
)
target_include_directories(solar_core PUBLIC src)
target_link_libraries(solar_core PUBLIC nlohmann_json::nlohmann_json spdlog::spdlog)
target_compile_definitions(solar_core PRIVATE SOLAR_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
solar_set_project_warnings(solar_core)
solar_enable_sanitizers(solar_core)

add_library(solar_astronomy
    src/astronomy/BodyDatabase.cpp
    src/astronomy/CelestialBody.cpp
    src/astronomy/Ephemeris.cpp
    src/astronomy/Orientation.cpp
    src/astronomy/ReferenceFrame.cpp
    src/astronomy/SpiceContext.cpp
)
target_include_directories(solar_astronomy PUBLIC src)
target_link_libraries(solar_astronomy PUBLIC solar_core glm::glm PRIVATE cspice::cspice)
solar_set_project_warnings(solar_astronomy)
solar_enable_sanitizers(solar_astronomy)

add_library(solar_app_config src/app/AppConfig.cpp)
target_include_directories(solar_app_config PUBLIC src)
target_link_libraries(solar_app_config PUBLIC solar_astronomy solar_core nlohmann_json::nlohmann_json)
solar_set_project_warnings(solar_app_config)
solar_enable_sanitizers(solar_app_config)

add_library(solar_astrodynamics
    src/astrodynamics/Angles.cpp
    src/astrodynamics/Hohmann.cpp
    src/astrodynamics/Kepler.cpp
    src/astrodynamics/Maneuver.cpp
    src/astrodynamics/ManeuverFrame.cpp
    src/astrodynamics/Orbit.cpp
    src/astrodynamics/OrbitConversions.cpp
    src/astrodynamics/OsculatingOrbit.cpp
    src/astrodynamics/OrbitalElements.cpp
    src/astrodynamics/OrbitalMechanics.cpp
    src/astrodynamics/OrbitPropagation.cpp
    src/astrodynamics/PlaneChange.cpp
)
target_include_directories(solar_astrodynamics PUBLIC src)
target_link_libraries(solar_astrodynamics PUBLIC solar_astronomy solar_core glm::glm)
solar_set_project_warnings(solar_astrodynamics)
solar_enable_sanitizers(solar_astrodynamics)

add_library(solar_rocket
    src/rocket/Burn.cpp
    src/rocket/DeltaV.cpp
    src/rocket/Engine.cpp
    src/rocket/EngineDatabase.cpp
    src/rocket/MassFlow.cpp
    src/rocket/Propellant.cpp
    src/rocket/RocketAnalysis.cpp
    src/rocket/Stage.cpp
    src/rocket/TWR.cpp
    src/rocket/Vehicle.cpp
    src/rocket/VehicleDatabase.cpp
)
target_include_directories(solar_rocket PUBLIC src)
target_link_libraries(solar_rocket PUBLIC solar_astrodynamics solar_astronomy solar_core
    nlohmann_json::nlohmann_json)
solar_set_project_warnings(solar_rocket)
solar_enable_sanitizers(solar_rocket)

add_library(solar_simulation
    src/simulation/OrbitVisualization.cpp
    src/simulation/Spacecraft.cpp
    src/simulation/SpacecraftState.cpp
)
target_include_directories(solar_simulation PUBLIC src)
target_link_libraries(solar_simulation PUBLIC solar_astrodynamics solar_astronomy solar_core glm::glm)
solar_set_project_warnings(solar_simulation)
solar_enable_sanitizers(solar_simulation)

add_library(solar_scene
    src/scene/RenderTransform.cpp
    src/scene/Scene.cpp
    src/scene/SolarSystemScene.cpp
)
target_include_directories(solar_scene PUBLIC src)
target_link_libraries(solar_scene PUBLIC solar_astrodynamics solar_astronomy solar_core glm::glm)
solar_set_project_warnings(solar_scene)
solar_enable_sanitizers(solar_scene)

add_library(solar_renderer
    src/renderer/BodyPicker.cpp
    src/renderer/Camera.cpp
    src/renderer/CameraController.cpp
    src/renderer/Framebuffer.cpp
    src/renderer/GridRenderer.cpp
    src/renderer/Mesh.cpp
    src/renderer/OrbitRenderer.cpp
    src/renderer/OrbitalGeometryRenderer.cpp
    src/renderer/PlanetRenderer.cpp
    src/renderer/PrimitiveRenderer.cpp
    src/renderer/Renderer.cpp
    src/renderer/Shader.cpp
    src/renderer/SpacecraftRenderer.cpp
    src/renderer/TrajectoryRenderer.cpp
)
target_include_directories(solar_renderer PUBLIC src)
target_link_libraries(solar_renderer PUBLIC solar_core solar_scene solar_simulation glad_gl_core_46 glfw glm::glm)
solar_set_project_warnings(solar_renderer)
solar_enable_sanitizers(solar_renderer)

add_library(solar_ui
    src/ui/ConsolePanel.cpp
    src/ui/AstronomyPanel.cpp
    src/ui/BodyInspectorPanel.cpp
    src/ui/Dockspace.cpp
    src/ui/Gui.cpp
    src/ui/InspectorPanel.cpp
    src/ui/ManeuverPanel.cpp
    src/ui/OrbitPanel.cpp
    src/ui/RocketPanel.cpp
    src/ui/StagePanel.cpp
    src/ui/EnginePanel.cpp
    src/ui/SolarSystemPanel.cpp
    src/ui/SpacecraftPanel.cpp
    src/ui/TimePanel.cpp
    src/ui/ViewportPanel.cpp
)
target_include_directories(solar_ui PUBLIC src)
target_link_libraries(solar_ui PUBLIC solar_core solar_rocket solar_scene solar_simulation solar_renderer imgui implot)
solar_set_project_warnings(solar_ui)
solar_enable_sanitizers(solar_ui)

add_executable(solar
    src/main.cpp
    src/app/Application.cpp
    src/app/AppState.cpp
)
target_include_directories(solar PRIVATE src)
target_link_libraries(solar PRIVATE solar_core solar_astronomy solar_astrodynamics solar_rocket solar_app_config solar_scene solar_simulation solar_renderer solar_ui)
solar_set_project_warnings(solar)
solar_enable_sanitizers(solar)

set_target_properties(solar PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

foreach(resource IN ITEMS shaders config assets)
    add_custom_command(TARGET solar POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/${resource}"
                "$<TARGET_FILE_DIR:solar>/${resource}"
        VERBATIM
    )
endforeach()

if(SOLAR_BUILD_TESTS)
    enable_testing()
    add_executable(solar_tests
        tests/test_main.cpp
        tests/test_body_database.cpp
        tests/test_body_picker.cpp
        tests/test_camera.cpp
        tests/test_camera_input.cpp
        tests/test_config.cpp
        tests/test_ephemeris.cpp
        tests/test_hohmann.cpp
        tests/test_kepler.cpp
        tests/test_maneuver.cpp
        tests/test_orbit_conversions.cpp
        tests/test_orbital_mechanics.cpp
        tests/test_planetary_orbits.cpp
        tests/test_plane_change.cpp
        tests/test_propagation.cpp
        tests/test_reference_frames.cpp
        tests/test_engine.cpp
        tests/test_stage.cpp
        tests/test_vehicle.cpp
        tests/test_delta_v.cpp
        tests/test_twr.cpp
        tests/test_mass_flow.cpp
        tests/test_burn.cpp
        tests/test_rocket_analysis.cpp
        tests/test_simulation_clock.cpp
        tests/test_spice.cpp
        tests/test_spacecraft.cpp
        tests/test_time.cpp
    )
    target_link_libraries(solar_tests PRIVATE solar_core solar_astronomy solar_astrodynamics solar_rocket solar_app_config solar_scene solar_simulation solar_renderer Catch2::Catch2)
    target_compile_definitions(solar_tests PRIVATE SOLAR_TEST_KERNEL_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/kernels")
    solar_set_project_warnings(solar_tests)
    solar_enable_sanitizers(solar_tests)
    add_test(NAME solar_tests COMMAND solar_tests)
endif()
