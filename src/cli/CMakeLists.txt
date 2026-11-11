# tante: the command-line tool. It links tante_api and nothing else
# (ADR-0004, docs/adr/0004-library-first-target-structure.md).
add_executable(tante main.cpp)
# PRIVATE: `tante` uses tante_api but passes nothing of it on. PUBLIC would
# hand tante_api's include folder and library on to every target that links
# `tante`, and nothing links an executable. The rule for every link: PRIVATE
# unless a public header of the linking target needs the other target.
target_link_libraries(tante PRIVATE tante_api)
tante_apply_guardrails(tante)
