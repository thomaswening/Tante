#pragma once

#include <string_view>

namespace tante {

/// Tante's version as "major.minor.patch", for example "0.1.0". Set in the
/// top-level CMakeLists.txt.
[[nodiscard]] std::string_view version() noexcept;

}  // namespace tante
