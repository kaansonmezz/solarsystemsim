# CMake generated Testfile for 
# Source directory: /home/garth/solar
# Build directory: /home/garth/solar/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test("solar_tests" "/home/garth/solar/build/solar_tests")
set_tests_properties("solar_tests" PROPERTIES  _BACKTRACE_TRIPLES "/home/garth/solar/CMakeLists.txt;215;add_test;/home/garth/solar/CMakeLists.txt;0;")
subdirs("_deps/glfw-build")
subdirs("_deps/glm-build")
subdirs("_deps/spdlog-build")
subdirs("_deps/nlohmann_json-build")
subdirs("_deps/catch2-build")
