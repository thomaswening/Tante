# tante_api: the public C++ API (include/tante/). Everything the command-line
# tool does goes through it (ADR-0004, docs/adr/0004-library-first-target-structure.md).
#
# STATIC, as every engine target (ADR-0004 point 3): one compiler builds all of
# `tante`, so no C++ types cross a DLL boundary.
add_library(tante_api STATIC version.cpp)
target_include_directories(tante_api PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_definitions(tante_api PRIVATE TANTE_VERSION_STRING="${PROJECT_VERSION}")
tante_apply_guardrails(tante_api)
