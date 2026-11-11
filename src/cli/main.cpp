// The command-line tool. It holds no calculation logic: everything it does
// goes through tante_api (REQ-064, docs/requirements.md#req-064).

#include <cstddef>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <span>
#include <string_view>

#include "tante/version.hpp"

namespace {

// Exit codes, as in diff(1) and grep(1): 0 success, 2 wrong usage.
constexpr int kExitUsage = 2;

constexpr std::string_view kVersionOption{"--version"};

int print_version() {
  std::cout << std::format("tante {}\n", tante::version());
  return EXIT_SUCCESS;
}

int print_usage() {
  std::cerr << std::format("usage: tante {}\n", kVersionOption);
  return kExitUsage;
}

int run(std::span<char*> arguments) {
  if (arguments.size() == 1 && std::string_view{arguments.front()} == kVersionOption) {
    return print_version();
  }
  return print_usage();
}

}  // namespace

int main(int argc, char* argv[]) {
  // An exception that left main would end the program without a message.
  // Which exit status each kind of error gets is the record on errors' to
  // decide; until then every error is EXIT_FAILURE.
  try {
    // The first element is the program's own name.
    return run(std::span{argv, static_cast<std::size_t>(argc)}.subspan(1));
  } catch (const std::exception& error) {
    // Not std::format, which may throw itself: nothing would catch that.
    std::cerr << "tante: " << error.what() << '\n';
  } catch (...) {
    std::cerr << "tante: unknown error\n";
  }
  return EXIT_FAILURE;
}
