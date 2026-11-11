#include "tante/version.hpp"

namespace tante {
namespace {

// TANTE_VERSION_STRING is PROJECT_VERSION, passed in by src/api/CMakeLists.txt.
constexpr std::string_view kVersion{TANTE_VERSION_STRING};

}  // namespace

std::string_view version() noexcept { return kVersion; }

}  // namespace tante
