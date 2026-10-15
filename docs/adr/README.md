# Architecture Decision Records

Each file records one significant decision: its context, the decision, the alternatives considered and the consequences. Records are numbered and never rewritten. If a decision changes, a new record supersedes the old one, and the old one's status is updated to point to it.

Status values: **Proposed** (direction chosen, not yet validated), **Accepted**, **Superseded by ADR-NNNN**.

A record that settles an open decision of [requirements.md](../requirements.md) names it in its title and links it, and the decision's entry in the requirements points back here. Evidence that was measured rather than argued lives in spikes (`docs/spikes/`) and is linked from the record.

New records copy [template.md](template.md).

## Records

| ADR | Decision | Settles | Status |
|---|---|---|---|
| [0001](0001-cpp20-cmake-vcpkg-guardrails.md) | C++20, CMake and vcpkg on Windows, macOS and Linux from the first commit, with warnings as errors, sanitizers and static analysis; a C interface for compiled modules | [OPEN-E](../requirements.md#open-e) | Proposed |
| [0002](0002-no-input-from-symbolica.md) | Nothing in Tante draws on Symbolica; external benchmark tables are used without their Symbolica column | [OPEN-P](../requirements.md#open-p) | Proposed |
