# tante: the command-line tool. It links tante_api and nothing else
# (ADR-0004, docs/adr/0004-library-first-target-structure.md).
add_executable(tante main.cpp)
target_link_libraries(tante PRIVATE tante_api)
tante_apply_guardrails(tante)
