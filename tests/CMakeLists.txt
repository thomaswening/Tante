# The test suite. How to run it and what the labels mean:
# docs/development.md#building-and-testing
#
# Each Catch2 TEST_CASE becomes its own CTest test, and its tags become CTest
# labels, so `ctest -L REQ-064` runs every test of one requirement.
find_package(Catch2 3 CONFIG REQUIRED)
include(Catch)

add_executable(tante_tests api/test_version.cpp)
target_link_libraries(tante_tests PRIVATE tante_api Catch2::Catch2WithMain)
tante_apply_guardrails(tante_tests)
catch_discover_tests(tante_tests ADD_TAGS_AS_LABELS DISCOVERY_MODE PRE_TEST)

# The command-line tool itself, run as a user runs it.
add_test(NAME cli.version COMMAND tante --version)
set_tests_properties(cli.version PROPERTIES
  LABELS "cli;REQ-064"
  PASS_REGULAR_EXPRESSION "^tante ${PROJECT_VERSION}\n$")

add_test(NAME cli.usage COMMAND tante --no-such-option)
set_tests_properties(cli.usage PROPERTIES
  LABELS "cli"
  WILL_FAIL TRUE)
