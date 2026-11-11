# Tante's build description. How to build and test: docs/development.md#building-and-testing
#
# The targets and the rules between them are ADR-0004's
# (docs/adr/0004-library-first-target-structure.md); the living list is
# docs/architecture.md#5-building-blocks. Each target has its own
# CMakeLists.txt in its directory. A target is added with its first code.
cmake_minimum_required(VERSION 3.28)

# The version `tante --version` prints. It becomes 0.1.0 with the engine
# preview (docs/roadmap.md).
project(tante VERSION 0.0.0 LANGUAGES CXX)

# ADR-0001 point 1 (docs/adr/0001-cpp20-cmake-vcpkg-guardrails.md): ISO C++20
# without compiler extensions.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(TanteCompilerOptions)

add_subdirectory(src/api)
add_subdirectory(src/cli)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
