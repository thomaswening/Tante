"""Tests of tools/check_architecture.py, the include check.

Run with: python -m unittest discover -s tools/tests
Each test builds a small repository in a temporary directory and checks it.
"""

from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

import check_architecture  # noqa: E402  (needs the path above)


class RepositoryFixture(unittest.TestCase):
    """A temporary repository with a vcpkg.json and the files a test writes into it."""

    def setUp(self) -> None:
        self._directory = tempfile.TemporaryDirectory()
        self.root = Path(self._directory.name)
        self.write_vcpkg_manifest(["catch2"])

    def tearDown(self) -> None:
        self._directory.cleanup()

    def write_vcpkg_manifest(self, dependencies: list) -> None:
        manifest = {"name": "tante", "dependencies": dependencies}
        (self.root / "vcpkg.json").write_text(json.dumps(manifest), encoding="utf-8")

    def write_source(self, relative_path: str, *include_lines: str) -> None:
        """A source file holding the given #include lines, refused outside the scanned directories."""
        if not relative_path.startswith(check_architecture.SCANNED_DIRECTORIES):
            raise ValueError(f"{relative_path} is outside the scanned directories, so the check "
                             "would never read it")
        path = self.root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("\n".join(include_lines) + "\n", encoding="utf-8")

    def findings(self) -> list[str]:
        return [str(finding) for finding in check_architecture.check_repository(self.root)]


class LibraryHeaderTests(RepositoryFixture):
    """REQ-096: only an adapter includes a third-party library's headers."""

    def test_req_096_gmp_in_the_integer_adapter_is_accepted(self) -> None:
        # Arrange
        self.write_source("src/arith/integer.cpp", "#include <gmp.h>")

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, [])

    def test_req_096_gmp_in_the_core_is_reported(self) -> None:
        # Arrange
        self.write_source("src/core/term.cpp", "#include <gmp.h>")

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, ["src/core/term.cpp:1: gmp.h is GMP's: only src/arith/ may "
                                    "include it"])

    def test_req_096_flint_in_its_adapter_is_accepted(self) -> None:
        # Arrange
        self.write_source("src/adapters/flint/backend.cpp", "#include <flint/fmpq_mpoly.h>")

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, [])

    def test_req_096_flint_in_tante_arith_is_reported(self) -> None:
        # Arrange
        self.write_source("src/arith/rational.cpp", "#include <flint/fmpz.h>")

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, ["src/arith/rational.cpp:1: flint/fmpz.h is FLINT's: only "
                                    "src/adapters/flint/ and src/adapters/arb/ may include it"])

    def test_req_096_mpfr_is_reported_everywhere(self) -> None:
        # Arrange
        self.write_source("src/adapters/arb/evaluate.cpp", "#include <mpfr.h>")

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, ["src/adapters/arb/evaluate.cpp:1: mpfr.h is MPFR's, which "
                                    "Tante does not include directly"])

    def test_req_050_a_system_header_outside_tante_platform_is_reported(self) -> None:
        # Arrange
        self.write_source("src/core/memory.cpp", "#include <windows.h>")

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, ["src/core/memory.cpp:1: windows.h is the operating system's: "
                                    "only src/platform/ may include it"])


class ProjectHeaderTests(RepositoryFixture):
    """The dependency rules between Tante's own targets."""

    def test_the_core_including_tante_arith_is_accepted(self) -> None:
        # Arrange
        self.write_source("src/core/term.cpp", '#include "arith/rational.hpp"')

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, [])

    def test_tante_arith_including_the_core_is_reported(self) -> None:
        # Arrange
        self.write_source("src/arith/rational.cpp", '#include "core/term.hpp"')

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, ["src/arith/rational.cpp:1: src/arith/ may not include "
                                    "core/term.hpp: it may include only arith/"])

    def test_req_064_the_command_line_tool_including_the_public_api_is_accepted(self) -> None:
        # Arrange
        self.write_source("src/cli/main.cpp", '#include "tante/version.hpp"')

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, [])

    def test_req_064_the_command_line_tool_including_the_core_is_reported(self) -> None:
        # Arrange
        self.write_source("src/cli/main.cpp", '#include "core/term.hpp"')

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, ["src/cli/main.cpp:1: src/cli/ may not include core/term.hpp: "
                                    "it may include only tante/"])

    def test_req_030_a_module_including_the_sdk_is_accepted(self) -> None:
        # Arrange
        self.write_source("modules/linalg/matrix.cpp", "#include <tante/module.hpp>")

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, [])

    def test_req_030_a_module_including_the_core_is_reported(self) -> None:
        # Arrange
        self.write_source("modules/linalg/matrix.cpp", '#include "core/term.hpp"')

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, ["modules/linalg/matrix.cpp:1: modules/ may not include "
                                    "core/term.hpp: it may include only tante/module"])

    def test_the_module_sdk_including_the_public_api_is_reported(self) -> None:
        # Arrange
        self.write_source("include/tante/module/handles.h", '#include "tante/version.hpp"')

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, ["include/tante/module/handles.h:1: include/tante/module/ may "
                                    "not include tante/version.hpp: it may include only "
                                    "tante/module"])

    def test_tests_including_any_target_are_accepted(self) -> None:
        # Arrange
        self.write_source("tests/core/test_term.cpp", '#include "core/term.hpp"')

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, [])

    def test_a_file_next_to_the_includer_is_accepted(self) -> None:
        # Arrange
        self.write_source("src/core/term.cpp", '#include "term.hpp"')

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, [])

    def test_a_path_that_climbs_out_of_its_directory_is_reported(self) -> None:
        # Arrange
        self.write_source("src/cli/main.cpp", '#include "../core/term.hpp"')

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, ["src/cli/main.cpp:1: ../core/term.hpp climbs out of its "
                                    "directory: include it by its path from include/ or src/"])


class UnknownHeaderTests(RepositoryFixture):
    """Every header is the standard library's, Tante's own or a listed library's."""

    def test_a_cpp_standard_header_is_accepted(self) -> None:
        # Arrange
        self.write_source("src/core/term.cpp", "#include <vector>")

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, [])

    def test_a_c_standard_header_is_accepted(self) -> None:
        # Arrange
        self.write_source("include/tante/module/handles.h", "#include <stdint.h>")

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, [])

    def test_an_unlisted_library_header_is_reported(self) -> None:
        # Arrange
        self.write_source("src/core/term.cpp", "#include <boost/container/small_vector.hpp>")

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, ["src/core/term.cpp:1: boost/container/small_vector.hpp is not "
                                    "a known header: add its library to LIBRARIES in "
                                    "tools/check_architecture.py"])

    def test_a_file_in_a_directory_without_a_rule_is_reported(self) -> None:
        # Arrange
        self.write_source("src/unplanned/thing.cpp", "#include <vector>")

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, ["src/unplanned/thing.cpp: no rule for this directory: add it "
                                    "to AREAS in tools/check_architecture.py"])

    def test_a_build_tree_inside_a_scanned_directory_is_skipped(self) -> None:
        # Arrange
        self.write_source("examples/module-hello/build/generated.cpp", "#include <windows.h>")

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, [])


class VcpkgManifestTests(RepositoryFixture):
    """Every library in vcpkg.json has a rule, so a new library cannot slip in unruled."""

    def test_a_dependency_without_a_library_entry_is_reported(self) -> None:
        # Arrange
        self.write_vcpkg_manifest(["catch2", "boost-container"])

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, ["vcpkg.json: dependency 'boost-container' has no entry in "
                                    "LIBRARIES in tools/check_architecture.py"])

    def test_a_dependency_written_as_an_object_is_recognised(self) -> None:
        # Arrange
        self.write_vcpkg_manifest(["catch2", {"name": "gmp", "default-features": False}])

        # Act
        findings = self.findings()

        # Assert
        self.assertEqual(findings, [])


class ExitStatusTests(RepositoryFixture):
    def test_a_clean_repository_exits_with_0(self) -> None:
        # Arrange
        self.write_source("src/core/term.cpp", "#include <vector>")

        # Act
        status = check_architecture.main([str(self.root)])

        # Assert
        self.assertEqual(status, check_architecture.EXIT_CLEAN)

    def test_a_repository_with_a_finding_exits_with_1(self) -> None:
        # Arrange
        self.write_source("src/core/term.cpp", "#include <gmp.h>")

        # Act
        status = check_architecture.main([str(self.root)])

        # Assert
        self.assertEqual(status, check_architecture.EXIT_FINDINGS)


if __name__ == "__main__":
    unittest.main()
