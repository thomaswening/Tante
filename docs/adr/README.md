# Architecture Decision Records

Each file records one significant decision: its context, the decision, the alternatives considered and the consequences. Records are numbered and never rewritten. If a decision changes, a new record supersedes the old one, and the old one's status is updated to point to it.

Status values: **Proposed** (direction chosen, not yet validated), **Accepted**, **Superseded by ADR-NNNN**. Each record is proposed in its own pull request and accepted by merging it, so `main` holds only accepted records; ADR-0001 to ADR-0003 predate this rule.

A record that settles an open decision of [requirements.md](../requirements.md) names it in its title and links it, and the decision's entry in the requirements points back here. Evidence that was measured rather than argued lives in spikes (`docs/spikes/`) and is linked from the record.

New records copy [template.md](template.md).

## Records

| ADR | Decision | Settles | Status |
|---|---|---|---|
| [0001](0001-cpp20-cmake-vcpkg-guardrails.md) | C++20, CMake and vcpkg on Windows, macOS and Linux from the first commit, with warnings as errors, sanitizers and static analysis; a C interface for compiled modules | [OPEN-E](../requirements.md#open-e) | Accepted |
| [0002](0002-no-input-from-symbolica.md) | Nothing in Tante draws on Symbolica; external benchmark tables are used without their Symbolica column | [OPEN-P](../requirements.md#open-p) | Accepted |
| [0003](0003-flint-gmp-arb-behind-own-interfaces.md) | Polynomial arithmetic through FLINT, integers through GMP, numerical evaluation through FLINT's ball arithmetic (arb), each behind an interface of Tante's own, with Tante's own simple polynomial implementation for cross-checks | [OPEN-O](../requirements.md#open-o) (a) | Accepted |
| [0004](0004-library-first-target-structure.md) | A library with a thin command-line tool, split into build targets whose dependencies the build and an include check enforce; every module, built-in or third-party, connects through the one C module interface | — | Accepted |
| [0005](0005-term-representation.md) | A term is a flat, pointer-free block of 32-bit words with a typed coefficient (rational number or rational function); an expression is a stream of terms behind a storage interface; only the core knows the layout, and built-in modules call it directly | — | Accepted |
