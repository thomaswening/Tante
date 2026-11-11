# tante_api: the public C++ API (include/tante/). Everything the command-line
# tool does goes through it (ADR-0004, docs/adr/0004-library-first-target-structure.md).
#
# STATIC: a static library (.lib, .a), copied into the executable when it is
# linked, as if the referenced assembly were merged into the .exe. ADR-0004
# point 3 makes every engine target static, so one compiler builds all of
# `tante`. A SHARED library (.dll) would export C++ types across a binary
# boundary whose layout differs between compilers.
add_library(tante_api STATIC version.cpp)
# PUBLIC: tante_api itself, and every target that links it, finds headers
# under include/, so `tante` can write #include "tante/version.hpp". PRIVATE
# would hide the folder from `tante`, and its build would fail.
target_include_directories(tante_api PUBLIC "${PROJECT_SOURCE_DIR}/include")
# PRIVATE: only version.cpp sees the macro. Linking targets do not get it, so
# no other code can come to depend on it.
target_compile_definitions(tante_api PRIVATE TANTE_VERSION_STRING="${PROJECT_VERSION}")
# Warnings and sanitizers: cmake/TanteCompilerOptions.cmake.
tante_apply_guardrails(tante_api)
