# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/garth/solar/build-test/_deps/cspice-src")
  file(MAKE_DIRECTORY "/home/garth/solar/build-test/_deps/cspice-src")
endif()
file(MAKE_DIRECTORY
  "/home/garth/solar/build-test/_deps/cspice-build"
  "/home/garth/solar/build-test/_deps/cspice-subbuild/cspice-populate-prefix"
  "/home/garth/solar/build-test/_deps/cspice-subbuild/cspice-populate-prefix/tmp"
  "/home/garth/solar/build-test/_deps/cspice-subbuild/cspice-populate-prefix/src/cspice-populate-stamp"
  "/home/garth/solar/build-test/_deps/cspice-subbuild/cspice-populate-prefix/src"
  "/home/garth/solar/build-test/_deps/cspice-subbuild/cspice-populate-prefix/src/cspice-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/garth/solar/build-test/_deps/cspice-subbuild/cspice-populate-prefix/src/cspice-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/garth/solar/build-test/_deps/cspice-subbuild/cspice-populate-prefix/src/cspice-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
