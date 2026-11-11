#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <string>

#include "tante/version.hpp"

TEST_CASE("the version has the form major.minor.patch", "[api][REQ-064]") {
  // Act
  const std::string version{tante::version()};

  // Assert
  CHECK_THAT(version, Catch::Matchers::Matches(R"(\d+\.\d+\.\d+)"));
}
