# Tante: Development

How work on Tante is done and checked: commits, documents, building and testing, and the checks that CI runs.

## Commits and pull requests

Every commit follows [Conventional Commits](https://www.conventionalcommits.org/) (`type(scope): summary`), so the changelog can be generated. Nothing is committed to `main` directly: every change, document or code, goes through a pull request that Thomas reviews and merges.

## Writing documentation

Decided by Thomas, 25 October 2026.

- **Two layers.** Project records (requirements, [ADRs](glossary.md#adr), [spikes](glossary.md#spike), architecture, roadmap) live in `docs/`. User documentation will live in `docs/user/`, sorted into tutorials, how-to guides, reference and explanation ([Diátaxis](https://diataxis.fr/)). Each subfolder is created with its first document.
- **Terms.** A term of art is linked to the [glossary](glossary.md) on its first use in each document, e.g. `[limb](glossary.md#limb)`. A new term gets a glossary entry in the same pull request that first uses it. <!-- glossary-ignore: term -->
- **Mathematics in LaTeX.** Formulas are written in LaTeX, which GitHub renders: inline as `$R_{\mu\nu} = 0$`, displayed between `$$` lines. Plain text is kept for code and for FORM or Tante input.
- **Language.** British English, sentence-case headings, absolute dates ("21 October 2026"), short declarative sentences.
- **Links.** Requirements, open decisions, ADRs and other documents are referred to by links, never by bare IDs or paths.
- **Diagrams.** Architecture, data layouts and workflows get a Mermaid diagram. Measured results get a chart.

## Building and testing

Tante is built with CMake and gets its libraries through [vcpkg](glossary.md#vcpkg), as [ADR-0001](adr/0001-cpp20-cmake-vcpkg-guardrails.md) decided. Each way of building is a [preset](glossary.md#preset) in `CMakePresets.json`, and its build tree is `build/<preset>/`.

### What you need

| | Windows | Linux | macOS |
|---|---|---|---|
| Compiler | Visual Studio 2026 with "Desktop development with C++" ([MSVC](glossary.md#msvc) 19.51) | [GCC](glossary.md#gcc) 13 or later | Xcode 26 (Apple Clang 21) |
| CMake | 3.28 or later. Visual Studio's own copy will do. | 3.28 or later | 3.28 or later |
| vcpkg | a clone at the commit in `vcpkg.json`'s `builtin-baseline`, with `VCPKG_ROOT` pointing at it | same | same |

Get vcpkg once, anywhere outside the repository:

```bash
git clone https://github.com/microsoft/vcpkg "$HOME/vcpkg"
git -C "$HOME/vcpkg" checkout c52f7bd748185ef0e0daf033d9f2ddfc8a3449ea
"$HOME/vcpkg/bootstrap-vcpkg.sh" -disableMetrics
```

On Windows run `bootstrap-vcpkg.bat` instead, and set `VCPKG_ROOT` in the system's environment variables. The first configure builds the libraries in `vcpkg.json`. Later configures take them from vcpkg's cache.

### Presets

| Preset | Build type | For |
|---|---|---|
| `windows-msvc-debug`, `linux-gcc-debug`, `macos-clang-debug` | Debug | daily work and the debugger |
| `windows-msvc-release`, `linux-gcc-release`, `macos-clang-release` | Release | what CI tests and releases ship |
| `windows-msvc-asan`, `linux-gcc-asan`, `macos-clang-asan` | RelWithDebInfo with [sanitizers](glossary.md#sanitizer) | finding undefined behaviour; [below](#sanitizers) |
| `linux-clang-tidy` | Debug with Clang | only the compile commands clang-tidy reads |

Every preset turns warnings into errors. To try a newer compiler without being blocked by its new warnings, add `-DCMAKE_COMPILE_WARNING_AS_ERROR=OFF` to the configure command. CI never does.

On Windows, vcpkg builds for the [triplet](glossary.md#triplet) `x64-windows-static-md`: libraries are linked statically, as vcpkg does by default on Linux and macOS, so no DLL has to sit next to `tante.exe`.

### Build and test

```bash
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug
```

Replace `linux-gcc-debug` with the preset for your system. Visual Studio, VS Code and CLion read `CMakePresets.json` themselves: open the folder and choose a preset.

The tests are one Catch2 program, `tante_tests`, plus tests that run the `tante` executable itself. Each Catch2 tag becomes a CTest label, so `ctest --preset linux-gcc-debug -L REQ-064` runs the tests of one requirement. A test names the requirements it checks in its tags, for example `[api][REQ-064]` ([REQ-072](requirements.md#req-072)). A test file tests one source file and is named after it: `src/api/version.cpp` is tested in `tests/api/test_version.cpp`.

### Sanitizers

A sanitizer makes the compiler add checks to the program, which stop it at the first undefined behaviour: reading past the end of an array, using freed memory, a signed overflow. Where .NET would throw an exception, C++ promises nothing, so these checks are how such a bug shows up at all. The `asan` presets turn on AddressSanitizer and, with GCC and Clang, UndefinedBehaviorSanitizer. The whole test suite runs under them in CI.

On Windows the AddressSanitizer runtime is a DLL that Visual Studio ships but does not put on `PATH`. Running the tests outside Visual Studio needs its folder on `PATH` first:

```powershell
$vs = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
$dll = Get-ChildItem "$vs\VC\Tools\MSVC" -Recurse -Filter clang_rt.asan_dynamic-x86_64.dll | Where-Object FullName -like '*Hostx64\x64*' | Select-Object -First 1
$env:PATH = "$($dll.DirectoryName);$env:PATH"
ctest --preset windows-msvc-asan
```

MSVC's AddressSanitizer is built with its container checks off: the vcpkg libraries are not built with the sanitizer, and the linker refuses to mix the two. Out-of-bounds access, use after free and leaks are still caught. The check for reading the unused capacity of a `std::vector` is lost on Windows only. Linux and macOS keep it.

## Tools and checks

### Continuous integration

[ci.yml](../.github/workflows/ci.yml) builds and tests Tante on Windows, macOS and Linux on every pull request and every push to `main`. It also runs clang-tidy, clang-format and the [include check](glossary.md#include-check). A pull request with a failed job cannot be merged. What each job wants and what to do when it fails: [tools/ci.md](tools/ci.md).

### Include check

[tools/check_architecture.py](../tools/check_architecture.py) fails CI on any `#include` that a directory may not use, which keeps the [dependency rules](architecture.md#dependency-rules) true. Its rules, cases and fixes: [tools/check_architecture.md](tools/check_architecture.md).

### Docs check

[docs-check.yml](../.github/workflows/docs-check.yml) runs `tools/check_docs.py` on every pull request: links and anchors, ADR status, Mermaid diagrams, the requirements' version line, glossary coverage, and the requirement IDs named in code and tests. It is a copy from claude-config's project-docs skill. The copy's first line names the version documented there.

```bash
python tools/check_docs.py . --base origin/main
```

### Weekly external-link check

Every Monday at 06:00 UTC, [docs-weekly.yml](../.github/workflows/docs-weekly.yml) runs `tools/check_docs.py --main` on `main`. Then [lychee](https://github.com/lycheeverse/lychee) follows every external link in `README.md` and `docs/`. A failure blocks nothing. It opens the issue "Weekly docs check failed", or comments on the one still open. <!-- glossary-ignore: block -->

**Rule: every external link answers with a success status within 60 seconds, unless [lychee.toml](../lychee.toml) excludes it.** Cited pages move or vanish without anyone changing the repository. Some sites refuse automated requests although the page is there. Those are excluded by pattern in `lychee.toml`, each with the reason. Decided by Thomas, 9 November 2026.

| Link | Result | Why |
|---|---|---|
| a page that answers 200, also after a redirect | accepted | |
| a page that answers 404, or a host that stays unreachable | reported | the citation is broken |
| `https://doi.org/10.1103/…` (APS) | excluded | the publisher answers 403 to automated requests |
| `https://github.com/…` | accepted | the workflow passes a token, so GitHub's limit for anonymous requests does not apply |
| a slow host that answers within 60 seconds | accepted | three retries |
| `https://gmplib.org/…`, `https://www.nikhef.nl/…` | excluded | the host never answers GitHub's runners, although it answers a browser |

**When it fails.** Open each reported link in a browser.

- The page is gone: link its new address. If there is none, link an archived copy from the [Wayback Machine](https://web.archive.org/) and say so in the citation, as for Ruijl's 2023 slides in [prior art](prior-art.md).
- The page opens in a browser but lychee gets 403 every week: add a pattern for that site to `lychee.toml`, with the reason and the date it was checked.
- The host times out week after week but opens in a browser within seconds: it drops requests from GitHub's runners. Add a pattern to `lychee.toml` as for a 403.
- The failure is a one-off timeout or 5xx: wait for the next week. Excluding it would hide a real breakage later.

Close the issue once the run is clean. The run can be started by hand: Actions → docs-weekly → Run workflow.

**Running it locally** needs [lychee](https://github.com/lycheeverse/lychee#installation):

```bash
lychee --no-progress "README.md" "docs/**/*.md"
```
