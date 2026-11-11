#!/usr/bin/env python3
"""The include check: every #include in Tante's C++ code is one its directory may use.

ADR-0004 point 5 (docs/adr/0004-library-first-target-structure.md) decided it.
vcpkg installs every library's headers into one folder, so the build graph alone
cannot keep FLINT's headers away from code that may use GMP. The rules and what
to do when the check fails: docs/development.md#include-check

Usage:
    python tools/check_architecture.py [repo-root]

Exit status: 0 when every include is allowed, 1 when any is not, 2 on wrong usage.
"""

from __future__ import annotations

import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path

EXIT_CLEAN = 0
EXIT_FINDINGS = 1
EXIT_USAGE = 2  # as in diff(1) and grep(1)

# The directories whose files are checked, and the file kinds in them.
SCANNED_DIRECTORIES = ("include", "src", "modules", "examples", "tests")
SOURCE_SUFFIXES = frozenset({".h", ".hpp", ".c", ".cpp"})
# A build tree inside a scanned directory, e.g. examples/module-hello/build/.
SKIPPED_DIRECTORY_NAMES = frozenset({"build"})

# The module SDK's include prefix: tante/module.hpp and include/tante/module/.
SDK = "tante/module"
# An area that may include every project header: the tests test all targets.
EVERY_PROJECT_HEADER = ("*",)

# Every prefix that names a header of Tante's own. The internal targets are
# included by their path under src/, e.g. "core/term_view.hpp".
PROJECT_PREFIXES = (
    "tante/", "api/", "arith/", "adapters/", "platform/", "core/", "lang/", "form/",
)


@dataclass(frozen=True)
class Area:
    """A directory and the project headers its files may include.

    The longest matching directory decides, so include/tante/module/ has its own
    rule inside include/tante/.
    """

    directory: str
    may_include: tuple[str, ...]


# The dependency rules of docs/architecture.md#dependency-rules, drawn as a
# diagram in docs/tools/check_architecture.md. An arrow in its
# diagram means "may use", and so does a chain of arrows, except where a rule
# says "only":
# - the command-line tool: REQ-064, docs/requirements.md#req-064
# - the modules: REQ-030, docs/requirements.md#req-030
# - the adapters: REQ-096, docs/requirements.md#req-096
AREAS = (
    Area("include/tante/module/", (SDK,)),
    Area("include/tante/", ("tante/",)),
    Area("src/cli/", ("tante/",)),
    Area("src/api/", ("tante/", "api/", "lang/", "form/", "core/", "arith/", "platform/", "adapters/")),
    Area("src/lang/", ("lang/", "core/", "arith/", "platform/", SDK)),
    Area("src/form/", ("form/", "core/", "arith/", "platform/", SDK)),
    Area("src/core/", ("core/", "arith/", "platform/", SDK)),
    Area("src/adapters/flint/", ("adapters/flint/", "arith/")),
    Area("src/adapters/simple/", ("adapters/simple/", "arith/")),
    Area("src/adapters/arb/", ("adapters/arb/", "arith/")),
    Area("src/arith/", ("arith/",)),
    Area("src/platform/", ("platform/",)),
    Area("modules/", (SDK,)),
    Area("examples/", (SDK,)),
    Area("tests/", EVERY_PROJECT_HEADER),
)


@dataclass(frozen=True)
class Library:
    """A third-party library: how its headers are recognised and who may include them."""

    name: str
    header_prefixes: tuple[str, ...]
    allowed_directories: tuple[str, ...]
    vcpkg_port: str | None


# REQ-096 (docs/requirements.md#req-096): only an adapter includes a library's
# headers. REQ-050 (docs/requirements.md#req-050): only tante_platform includes
# a system's own headers.
LIBRARIES = (
    Library("GMP", ("gmp.h", "gmpxx.h"), ("src/arith/",), "gmp"),
    Library("FLINT", ("flint/",), ("src/adapters/flint/", "src/adapters/arb/"), "flint"),
    # MPFR is a dependency of FLINT, which Tante does not call directly:
    # ADR-0003 §1, docs/adr/0003-flint-gmp-arb-behind-own-interfaces.md
    Library("MPFR", ("mpfr.h", "mpf2mpfr.h"), (), "mpfr"),
    Library("Catch2", ("catch2/",), ("tests/",), "catch2"),
    Library(
        "the operating system",
        ("windows.h", "dlfcn.h", "unistd.h", "sys/", "mach-o/"),
        ("src/platform/",),
        None,
    ),
)

# The C standard library's headers, which the C module interface needs. C++
# code includes the <cstdint> forms, which the "no dot, no slash" rule accepts.
C_STANDARD_HEADERS = frozenset({
    "stddef.h", "stdint.h", "stdbool.h", "stdlib.h", "string.h", "assert.h",
})

INCLUDE_PATTERN = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]')


@dataclass(frozen=True)
class Finding:
    """One include that breaks a rule, or one rule that is missing."""

    path: str
    line_number: int | None
    message: str

    def __str__(self) -> str:
        location = self.path if self.line_number is None else f"{self.path}:{self.line_number}"
        return f"{location}: {self.message}"


@dataclass(frozen=True)
class Include:
    """One #include line, e.g. line 3 of src/core/term.cpp: #include "arith/rational.hpp"."""

    path: str  # the including file, relative to the repository: "src/core/term.cpp"
    line_number: int  # counted from 1: 3
    header: str  # what is between the quotes or brackets: "arith/rational.hpp"
    is_quoted: bool  # True for "...", False for <...>


def check_repository(root: Path) -> list[Finding]:
    """Every finding in the repository at `root`, in file and line order."""
    findings = check_vcpkg_dependencies(root)
    for source_file in source_files(root):
        findings.extend(check_file(root, source_file))
    return findings


def check_vcpkg_dependencies(root: Path) -> list[Finding]:
    """A finding for each vcpkg.json dependency that no Library describes.

    Without this, a new library's headers would pass as unknown until someone
    noticed, and nobody would decide who may include them.
    """
    manifest = root / "vcpkg.json"
    if not manifest.exists():
        return []
    known_ports = {library.vcpkg_port for library in LIBRARIES}
    dependencies = json.loads(manifest.read_text(encoding="utf-8")).get("dependencies", [])
    names = [dependency_name(dependency) for dependency in dependencies]
    return [
        Finding("vcpkg.json", None,
                f"dependency '{name}' has no entry in LIBRARIES in tools/check_architecture.py")
        for name in names
        if name not in known_ports
    ]


def dependency_name(dependency: str | dict) -> str:
    """A vcpkg dependency is a port name or an object with a "name"."""
    return dependency if isinstance(dependency, str) else dependency["name"]


def source_files(root: Path) -> list[Path]:
    """The C and C++ files under the scanned directories, sorted, build trees left out."""
    candidates = [
        path
        for directory in SCANNED_DIRECTORIES
        for path in (root / directory).rglob("*")
        if path.suffix in SOURCE_SUFFIXES and path.is_file()
    ]
    return sorted(path for path in candidates if not is_in_build_tree(root, path))


def is_in_build_tree(root: Path, path: Path) -> bool:
    return any(part in SKIPPED_DIRECTORY_NAMES for part in path.relative_to(root).parts)


def check_file(root: Path, source_file: Path) -> list[Finding]:
    relative_path = source_file.relative_to(root).as_posix()
    area = area_of(relative_path)
    if area is None:
        return [Finding(relative_path, None,
                        "no rule for this directory: add it to AREAS in tools/check_architecture.py")]
    includes = includes_in(relative_path, source_file.read_text(encoding="utf-8"))
    findings = [check_include(include, area) for include in includes]
    return [finding for finding in findings if finding is not None]


def area_of(relative_path: str) -> Area | None:
    """The area with the longest directory that contains the file."""
    matching = [area for area in AREAS if relative_path.startswith(area.directory)]
    return max(matching, key=lambda area: len(area.directory), default=None)


def includes_in(relative_path: str, text: str) -> list[Include]:
    matches = [
        (line_number, INCLUDE_PATTERN.match(line))
        for line_number, line in enumerate(text.splitlines(), start=1)
    ]
    return [
        Include(relative_path, line_number, match.group(2), match.group(1) == '"')
        for line_number, match in matches
        if match is not None
    ]


def check_include(include: Include, area: Area) -> Finding | None:
    """None if the area may include the header, else the finding that says why not."""
    header = include.header
    if header.startswith("../") or "/../" in header:
        return finding_at(include, f"{header} climbs out of its directory: include it by its "
                                   "path from include/ or src/")
    if include.is_quoted and "/" not in header:
        return None  # a file next to this one, part of the same target
    if header.startswith(PROJECT_PREFIXES):
        return check_project_include(include, area)
    library = library_of(header)
    if library is not None:
        return check_library_include(include, library)
    if is_standard_header(header):
        return None
    return finding_at(include, f"{header} is not a known header: add its library to LIBRARIES "
                               "in tools/check_architecture.py")


def check_project_include(include: Include, area: Area) -> Finding | None:
    if area.may_include == EVERY_PROJECT_HEADER or include.header.startswith(area.may_include):
        return None
    allowed = ", ".join(area.may_include)
    return finding_at(include, f"{area.directory} may not include {include.header}: it may "
                               f"include only {allowed}")


def check_library_include(include: Include, library: Library) -> Finding | None:
    if include.path.startswith(library.allowed_directories):
        return None
    if not library.allowed_directories:
        return finding_at(include, f"{include.header} is {library.name}'s, which Tante does not "
                                   "include directly")
    allowed = " and ".join(library.allowed_directories)
    return finding_at(include, f"{include.header} is {library.name}'s: only {allowed} may "
                               "include it")


def library_of(header: str) -> Library | None:
    return next((library for library in LIBRARIES if header.startswith(library.header_prefixes)),
                None)


def is_standard_header(header: str) -> bool:
    """C++ standard headers have no extension and no directory: <vector>, <cstdint>."""
    return ("." not in header and "/" not in header) or header in C_STANDARD_HEADERS


def finding_at(include: Include, message: str) -> Finding:
    return Finding(include.path, include.line_number, message)


def main(arguments: list[str]) -> int:
    if len(arguments) > 1:
        print(__doc__, file=sys.stderr)
        return EXIT_USAGE
    root = Path(arguments[0]) if arguments else Path(".")
    findings = check_repository(root.resolve())
    for finding in findings:
        print(finding)
    print(f"check_architecture: {len(findings)} finding(s)")
    return EXIT_FINDINGS if findings else EXIT_CLEAN


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
