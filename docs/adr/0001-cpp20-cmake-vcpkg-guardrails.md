# ADR-0001: Tante is written in C++20, built with CMake and vcpkg on all three systems from the first commit, under compiler-enforced guardrails (resolves OPEN-E)

- **Status:** Proposed
- **Date:** 2026-10-16

## Context

[OPEN-E](../requirements.md#open-e) asked for the implementation language. Thomas resolved it on 16 October 2026: **C++20**. This record writes that decision down and settles what it leaves open: how Tante is built on Windows, macOS and Linux, how its arithmetic libraries are obtained, how a maintainer still learning C++ is protected from the language's sharp edges, and what the C interface for compiled modules looks like.

Requirements involved:

- [REQ-050](../requirements.md#req-050): native 64-bit builds on Windows, macOS and Linux, identical output on all three, no administrator rights to install.
- [REQ-051](../requirements.md#req-051): unattended batch runs on Linux clusters.
- [REQ-074](../requirements.md#req-074): byte-identical output across threads, machines and runs.
- [REQ-096](../requirements.md#req-096): every external library behind an interface of Tante's own. The libraries expected are GMP (integers), FLINT (polynomials) and MPFR (floating point, [REQ-101](../requirements.md#req-101)); the choice of FLINT is a separate record.
- [REQ-032](../requirements.md#req-032): compiled modules connect through a C interface that stays stable within a major version of [REQ-031](../requirements.md#req-031).
- [REQ-061](../requirements.md#req-061) to [REQ-064](../requirements.md#req-064): a C++ library, a Python package installable without a compiler, notebooks, and a library-first structure.
- [REQ-080](../requirements.md#req-080) and [REQ-092](../requirements.md#req-092): one maintainer, and a codebase a newcomer can change without help.

Evidence:

- The open-licensed engines that reach the largest scales (FORM, GiNaC, SymEngine, Cadabra2) are written in C or C++, and FLINT, GMP and MPFR are C libraries, so C++ calls them with no translation layer ([prior-art.md §4.6](../prior-art.md#46-implementation-language-and-platforms-open-e-req-050)).
- The survey found that the risks that end projects of this kind are a single maintainer (Package-X, HEPMath) and an aging toolchain (Redberry), not the language (same section).
- FORM 5 deprecated its native Windows builds because maintaining them from Linux was costly, and it now ships Windows binaries built through MSYS2. Cadabra2 ships natively on Windows, also through MSYS2's UCRT64 environment (same section). Native Windows is therefore achievable. The cost falls on whoever does not test it continuously.
- vcpkg, Microsoft's open-source C++ package manager, has ports of FLINT 3.6.0, GMP 6.3.0 and MPFR 4.2.2. The FLINT port is supported on 64-bit Windows, Linux and macOS, but not on Windows on ARM (checked at vcpkg.io on 16 October 2026). vcpkg's GMP port keeps GMP's hand-written assembly when building with MSVC on x64, assembling it with Clang; it disables assembly only on Windows on ARM ([portfile at the pinned baseline](https://github.com/microsoft/vcpkg/blob/c52f7bd748185ef0e0daf033d9f2ddfc8a3449ea/ports/gmp/portfile.cmake)).
- The [native arithmetic build spike](../spikes/2026-10-16-native-arithmetic-build.md) of 16 October 2026 (§6 below) built all three libraries through vcpkg with MSVC, GCC and Apple Clang on GitHub's runners and got byte-identical results on the three systems.

Thomas's stated position: Rust was rejected because he does not know it and would be learning a hard language while maintaining the engine alone. C# was rejected because it is generally slower than C++ for this work, and because researchers neither use nor know it, which matters for adoption and for a future maintainer. He wants to learn C++, and the project is the occasion. This record adopts that position. Most of what follows is about making a learner's C++ safe.

## Decision

**Tante is written in ISO C++20, built with CMake, gets its third-party libraries through vcpkg in manifest mode, and is built and tested on Windows, macOS and Linux on every change from the first commit, with warnings as errors, sanitizers and static analysis enforced in CI.** The parts:

### 1. Language level

- The standard is C++20, set in CMake with `CMAKE_CXX_STANDARD 20` and `CMAKE_CXX_EXTENSIONS OFF`, so that compiler-specific extensions are not used by accident. A move to C++23 is a later record, once all three compilers support what it would use.
- **Supported compilers are exactly those CI builds with** (Thomas, 16 October 2026): no compiler is promised that is not tested on every change. At the spike that meant MSVC 19.51 (Visual Studio 2026) on Windows, GCC 13.3 on Linux (the first GCC with `std::format`) and Apple Clang 21 (Xcode 26) on macOS. When GitHub updates a runner's compiler, the supported version moves with it; supporting an older compiler means adding it to CI first.
- C++20 features Tante will use: **concepts** (constraints on template parameters, comparable to C# generic constraints `where T : IComparable<T>`, but checked when the template is used); **ranges** and `std::span` (a non-owning view of a contiguous array, like `Span<T>`); **three-way comparison** `operator<=>` (one function that defines `<`, `==` and `>`, much like implementing `IComparable<T>.CompareTo`), which suits the canonical order of [REQ-012](../requirements.md#req-012); `std::format`; `<bit>` for fixed-width bit manipulation.
- Not used for now: **C++20 named modules** (`import tante;`). Support for them in CMake and the three compilers is still uneven, and the name collides with Tante's own "modules" ([REQ-030](../requirements.md#req-030)). Code is organised in ordinary headers and source files. Coroutines are also not used for now.

### 2. Build and dependencies

- **CMake** (3.28 or later) is the build description, with a `CMakePresets.json` that names each configuration (for example `windows-msvc-debug`, `linux-gcc-asan`). A preset is to CMake roughly what a launch profile is to a .NET project. CMake also generates Visual Studio solutions, so Thomas can work in Visual Studio, VS Code or CLion.
- **vcpkg in manifest mode**: a `vcpkg.json` in the repository lists GMP, MPFR, FLINT and the test framework, with a pinned `builtin-baseline` so every machine and CI run builds the same versions. This plays the role of a `.csproj`'s `PackageReference` list with a lock file. vcpkg builds the libraries from source with the project's compiler, which is what makes the Windows build native rather than an MSYS2 port. Binary caching in CI keeps this from rebuilding FLINT on every run: the spike measured a cold configure of about 8 minutes on Linux, 6 on macOS and 49 on Windows, and 6 to 23 seconds with the cache.
- **Windows toolchain: MSVC**, the compiler Thomas already has with Visual Studio, and the one Windows developers and the later Python wheels ([REQ-062](../requirements.md#req-062)) expect. The fallback, if the spike below shows the MSVC build of GMP or FLINT is unusable or much slower, is clang-cl (Clang with MSVC compatibility), then MinGW-w64 UCRT64 (the route FORM and Cadabra2 take). Changing the Windows toolchain is a new record superseding this part.
- **Only 64-bit targets**, x86-64 on all three systems and ARM64 on macOS and Linux ([REQ-050](../requirements.md#req-050)). Windows on ARM is not a target while the FLINT port excludes it.
- **Test framework: Catch2 v3**, whose tags carry requirement IDs (`[REQ-012]`), as the project-inception checker expects. The tests are a separate CMake target, like a separate xUnit test project.

### 3. Guardrails for a maintainer learning C++

C++ allows programs that compile and then behave unpredictably: reading past the end of an array, using memory after it has been freed, integer overflow of signed types, data races between threads. The standard calls these **undefined behaviour**. .NET throws an exception in most of these situations; C++ makes no promise at all, and the program may appear to work. Every guardrail below exists to turn undefined behaviour into a failed build or a failed test.

- **Warnings are errors** in CI and in the default presets: MSVC `/W4 /WX /permissive- /utf-8`; GCC and Clang `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Werror`. `/permissive-` makes MSVC follow the standard instead of accepting older non-standard code. `-Wconversion` catches silent narrowing such as assigning a 64-bit value to a 32-bit variable, which matters for [REQ-050](../requirements.md#req-050): `long` is 32 bits on Windows and 64 bits on Linux and macOS, so Tante's own code uses fixed-width types (`std::int64_t`, `std::uint64_t`) and never `long`. Third-party headers are included as system headers so that their warnings do not fail the build.
- **Sanitizers** in CI, on every change: AddressSanitizer (out-of-bounds access, use after free, leaks) and UndefinedBehaviorSanitizer (signed overflow, invalid shifts, misaligned access) with Clang or GCC on Linux and macOS; AddressSanitizer with MSVC on Windows; ThreadSanitizer (data races) on Linux from the first multithreaded code ([REQ-052](../requirements.md#req-052)). The whole test suite runs under them.
- **Static analysis**: clang-tidy with a curated set of checks (`bugprone-*`, `cppcoreguidelines-*`, `modernize-*`, `performance-*`, `readability-*`, minus those that conflict with the project's style), run in CI and failing on findings. The **C++ Core Guidelines** are the style reference, and clang-tidy enforces parts of them.
- **Formatting**: clang-format with a committed `.clang-format`, checked in CI, so that formatting is never a review question.
- **Ownership by RAII.** Every resource (memory, files, library handles such as a FLINT polynomial) is owned by an object whose destructor releases it. The closest .NET analogy is a `using` block around an `IDisposable`, except that in C++ it is automatic and deterministic for every local variable. Code does not use raw `new` and `delete`: ownership is expressed with values, standard containers, `std::unique_ptr` (one owner) and, only where sharing is real, `std::shared_ptr`. The adapters for GMP, FLINT and MPFR wrap each C handle in such a class at the [REQ-096](../requirements.md#req-096) boundary.
- **Errors inside the engine are C++ exceptions**, as in .NET. They never cross the C interface (below). How errors reach the user ([REQ-024](../requirements.md#req-024)) is a separate record.
- **Determinism hazards are banned by review and, where possible, by tooling** ([REQ-074](../requirements.md#req-074)): no output order that depends on iterating an unordered container, on pointer values or on hashing. The record on canonical order states this rule precisely.

### 4. The C interface for compiled modules and bindings

- The engine exposes a **C interface**: a header `tante.h` that declares functions with C linkage (`extern "C"`), uses only C types (fixed-width integers, `const char*` in UTF-8, opaque handle types such as `tante_expr*`), and reports errors as status codes with a retrievable message. It is the C counterpart of a COM-style or P/Invoke-friendly API: C++ classes, templates, exceptions and standard-library types never cross it, because their binary layout differs between compilers and compiler versions, while C's does not ([REQ-032](../requirements.md#req-032)).
- A compiled module is a shared library (`.dll`, `.so`, `.dylib`) that exports one entry function, given a C table of engine functions and returning a C table of what the module provides, with the interface's major version checked first ([REQ-031](../requirements.md#req-031)). Its exact shape is the record on modules.
- The C++ library of [REQ-061](../requirements.md#req-061) is the engine's own C++ interface, used by the command-line tool ([REQ-064](../requirements.md#req-064)). It is source-compatible within a major version, not binary-compatible: users build it with their own compiler.
- The Python package of [REQ-062](../requirements.md#req-062) and the notebooks of [REQ-063](../requirements.md#req-063) are bindings built on one of the two interfaces and shipped as pre-built wheels for the three systems. Which binding generator is used (nanobind, pybind11 or the C interface through `ctypes`/`cffi`) is decided when that release is planned.

### 5. Continuous integration

- GitHub Actions builds and tests on `windows-latest`, `macos-latest` and `ubuntu-latest` on every push and pull request, from the first commit with code, plus the sanitizer, clang-tidy and clang-format jobs above. A change that fails on one system is not merged.
- Output determinism across the three systems ([REQ-050](../requirements.md#req-050), [REQ-074](../requirements.md#req-074) (a)) is tested by comparing the outputs of the three jobs byte for byte, once there is output to compare.

### 6. Spike before acceptance

Before this record is accepted, a spike `docs/spikes/YYYY-MM-DD-native-arithmetic-build.md` establishes, on the GitHub-hosted runners of all three systems:

1. Does a CMake project with a vcpkg manifest build GMP, MPFR and FLINT and link a C++20 test program that multiplies two multivariate polynomials and takes their GCD, with MSVC, GCC and Apple Clang?
2. Are GMP's limbs and FLINT's `ulong` 64 bits wide on all three, so that the same inputs give the same results?
3. Is the result printed identically on all three?
4. How long does the vcpkg build take, cold and with binary caching?
5. How much slower is the MSVC build of GMP and FLINT than the Linux build on a short polynomial benchmark (for example a few polybench GCD cases, without its Symbolica column, per [OPEN-P](../requirements.md#open-p)), and is the difference large enough to prefer clang-cl or MinGW-w64 on Windows?
6. Do AddressSanitizer and UndefinedBehaviorSanitizer run cleanly over that program on each system?

A failure of 1 to 3 on Windows sends the decision to the clang-cl or MinGW-w64 fallback. The answer to 5 does not reopen the language.

**Result** ([spike record](../spikes/2026-10-16-native-arithmetic-build.md), 16 October 2026): (1) yes, GMP 6.3.0, MPFR 4.2.2 and FLINT 3.6.0 built and linked with MSVC 19.51, GCC 13.3 and Apple Clang 21; (2) yes, 64-bit limbs and `ulong` everywhere, although C's `long` is 32 bits on Windows; (3) yes, `results.txt` byte-identical on the three systems, including the order of factors FLINT returned; (4) cold configure 486 s on Linux, 372 s on macOS and 2,954 s on Windows, warm 6 s, 8 s and 23 s; (5) on the same CPU, MSVC's builds were within about 5 % of GCC's on GMP multiplication and polynomial GCD and about 25 % slower on polynomial multiplication, well inside the runners' own variation, so **MSVC stays the Windows toolchain and the fallback is not needed**; (6) yes, AddressSanitizer and UndefinedBehaviorSanitizer reported nothing, though they instrumented only the spike's code, not the libraries.

## Alternatives considered

- **Rust:** strong guarantees against exactly the undefined behaviour the guardrails above work to catch, and Cargo is a better package manager than any C++ option. Rejected by Thomas: he does not know Rust and would be learning a hard language while maintaining the engine alone, and FLINT would be reached through bindings whose maturity the survey did not check ([prior-art.md §4.6](../prior-art.md#46-implementation-language-and-platforms-open-e-req-050)). This is the strongest alternative; the guardrails are the price of not choosing it.
- **C# on .NET:** the language Thomas knows best, memory-safe, with excellent tooling. Rejected by Thomas: generally slower than C++ for allocation-heavy symbolic work, and not a language researchers use or know, which matters for adoption and for a successor maintainer ([REQ-092](../requirements.md#req-092)).
- **C (as FORM):** the simplest binary interface, but no RAII, no generic containers and no exceptions, so more of the bookkeeping that C++ automates would be done by hand by a single maintainer. Not chosen.
- **C++17 or C++23 instead of C++20:** C++17 lacks concepts, `operator<=>` and `std::span`, which simplify the core types; C++23 is not yet fully supported by all three compilers. C++20 is supported by all three for the features listed.
- **Meson, Bazel or a hand-written Makefile instead of CMake:** Meson is cleaner, Bazel more reproducible, but CMake is what Visual Studio, CLion, VS Code, vcpkg, Conan, scikit-build-core (for the Python wheels) and most contributors expect. Not chosen.
- **Conan instead of vcpkg:** comparable; Conan also has GMP, MPFR and FLINT recipes. vcpkg was preferred for its integration with Visual Studio and MSVC, Thomas's environment. Revisitable without touching code, since both feed CMake's `find_package`.
- **System packages (apt, Homebrew, MSYS2 pacman):** easiest on Linux and macOS, but gives different library versions on each system and no native MSVC build on Windows, which undermines [REQ-050](../requirements.md#req-050)'s identical output. Allowed for local development on Linux, not for CI or releases.
- **Building on Windows through MSYS2 only, as FORM and Cadabra2 do:** proven, and the fallback above. Not the first choice because it puts Windows developers in a Unix-like environment, and Visual Studio's debugger and MSVC's AddressSanitizer would not apply.

## Consequences

- Easier: direct, zero-cost use of FLINT, GMP and MPFR; the C++ library of [REQ-061](../requirements.md#req-061) is the engine's own API; one build description for three systems; Thomas debugs in Visual Studio.
- Harder: the maintainer is learning C++ while writing the engine. The guardrails make mistakes visible but do not prevent them, and the first months will be slower than in C#. Code review should explain C++ idioms in the code's comments where a .NET developer would not expect them.
- Harder: three-system CI from the first commit costs CI minutes (macOS runners are the most expensive for private repositories) and slows every change slightly. Making the repository public, already needed for the reference machine ([OPEN-G](../requirements.md#open-g)), removes the minute limits for standard runners.
- Harder: a cold vcpkg build of GMP, MPFR and FLINT takes about 50 minutes on Windows and 6 to 8 on macOS and Linux; binary caching is needed in CI from the start.
- Risk: GMP's build tunes itself to the CPU of the machine that builds it, and vcpkg's `fat` option (runtime selection of routines) is not available with MSVC. A cached or released binary built on a newer CPU may fail on an older one. CI's cache key and the release builds must account for this, for example by building GMP for a generic x86-64 target in releases.
- Follow-up: the spike of §6 is done and recorded; this record awaits Thomas's review. The spike's CMake project and workflow stay on the branch `spike/native-arithmetic-build`; the project's own CMake skeleton and CI workflow are added with the first code, after Thomas approves them.
- Follow-up: `docs/development.md` (stage 6) records the build commands, presets, the clang-tidy check list and how to run the sanitizers locally.
- Follow-up: the records on the polynomial back end (FLINT, its licence and version), on modules and the C interface, and on error handling refine §2 and §4.
- Untested: the speed of MSVC's builds beyond the spike's four short benchmarks ([REQ-056](../requirements.md#req-056) is measured on the Linux reference machine, so Windows speed is not a requirement); whether FLINT's CMake build on Windows enables the same optimised code as its autotools build elsewhere; the libraries under the sanitizers; whether C++20 named modules become usable later.
