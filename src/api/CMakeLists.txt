# tante_api: the public C++ API (include/tante/). Everything the command-line
# tool does goes through it (ADR-0004, docs/adr/0004-library-first-target-structure.md).
add_library(tante_api STATIC version.cpp)
target_include_directories(tante_api PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_definitions(tante_api PRIVATE TANTE_VERSION_STRING="${PROJECT_VERSION}")
tante_apply_guardrails(tante_api)
