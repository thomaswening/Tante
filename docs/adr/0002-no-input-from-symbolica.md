# ADR-0002: Nothing in Tante draws on Symbolica; external benchmark tables are used without their Symbolica column (resolves OPEN-P)

- **Status:** Proposed
- **Date:** 2026-10-16

## Context

[OPEN-P](../requirements.md#open-p) asked what Tante does about Symbolica's licence. Thomas resolved it on 15 October 2026, as proposed: a project rule, recorded by ADR, that no requirement, design, test or benchmark of Tante draws on Symbolica, and that external benchmark tables such as polybench are used without their Symbolica column. This record writes that rule down precisely enough to follow and check.

Evidence ([prior-art.md §2.4](../prior-art.md#24-symbolica-positioning-and-licence-only-by-deliberate-choice)):

- Symbolica's Source-Available License 1.0 took effect on 15 September 2026 for its v3.0 series. It defines a "Competing Product" to include free and open-source software whose computer-algebra functionality is substantially similar to or substitutable for Symbolica's. A GPL-3.0 successor to FORM for physicists ([REQ-080](../requirements.md#req-080)) is very plausibly one.
- Its §4(4) forbids using the software, its source, binaries or "Derived Information" as a reference implementation, behavioural oracle, development blueprint or other development input for a Competing Product, including to design, test, validate, benchmark or document any algorithm or data structure. Its §4(5) forbids plans, specifications, test suites, benchmarks and output comparisons informed by those materials. "Software" includes its documentation, comments and examples. Its §5 forbids using AI systems to recover or infer implementation details from it, and the restrictions extend to clean-room teams.
- The licence carves out independent development that uses none of those materials.
- The survey did not verify whether releases before 15 September 2026 (up to 2.2.0) remain under their earlier terms.

Requirements involved: [REQ-004](../requirements.md#req-004) (a new, independent engine), [REQ-080](../requirements.md#req-080) (GPL-3.0-or-later, forks welcome), [REQ-056](../requirements.md#req-056) (published performance factors), [REQ-071](../requirements.md#req-071) (independent cross-checks) and [REQ-096](../requirements.md#req-096) (replaceable libraries: Symbolica's own change of licence is one of the reasons given for it).

This is a reading of the licence text by the project, not legal advice. Whether to take legal advice was not decided by Thomas and remains Thomas's decision.

## Decision

**No part of Tante — requirements, design records, architecture, code, tests, the reference corpus, benchmarks, documentation or release notes — is informed by Symbolica's source, binaries, documentation, examples, website, papers about its internals, benchmark numbers or output. External material that contains Symbolica results is used only with those results removed.** In practice:

1. **Not read for design.** Project members, and AI assistants working on the project, do not read Symbolica's source, documentation, examples, issue tracker or technical website pages for the purpose of Tante's work, and do not ask an AI system to summarise them. This applies to every Symbolica version, including those released before 15 September 2026, because the status of earlier releases was not verified and treating all versions alike is simpler to follow.
2. **Not run.** Symbolica is not installed, run or linked by the project, its CI, its benchmarks or its tests, and its output is never compared with Tante's.
3. **Benchmark tables without the Symbolica column.** When an external benchmark suite or table reports Symbolica alongside other systems, as [polybench](https://github.com/tueda/polybench/) does, Tante uses only the problem definitions and the other systems' columns. A vendored copy of such a table has the Symbolica column deleted before it is committed, and the commit message says so. Polybench's problem generators and its non-Symbolica results are safe sources (MIT licence, [prior-art.md §5](../prior-art.md#5-validation-corpus-catalogue)).
4. **Sources that are allowed.** FORM's papers and GPL-3.0 source, GiNaC, SymEngine, SymPy, Cadabra2, xAct, Redberry, FLINT, GMP, MPFR, the algorithmic literature, and polybench without its Symbolica column, as [prior-art.md §4.1](../prior-art.md#41-a-new-and-independent-engine-req-004) lists. Where FORM's own developers built on ideas later used in Symbolica, Tante draws on the FORM publications and FORM source, not on Symbolica.
5. **Public positioning only.** Facts about Symbolica that say nothing about how it works — its name, author, licence, language, release dates and prices — may be stated, as [prior-art.md §2.4](../prior-art.md#24-symbolica-positioning-and-licence-only-by-deliberate-choice) does, to explain why Tante exists and why this rule does. They are never used as a requirement or a design argument; [OPEN-E](../requirements.md#open-e) already records that Symbolica's choice of language was not used.
6. **Contributions and forks.** When the project accepts outside contributions ([REQ-080](../requirements.md#req-080)), the contributing guide states this rule, and a contribution that cites Symbolica as its source is declined. Until then the rule binds the maintainer and the tools the maintainer uses.
7. **Accidental exposure.** If Symbolica material is found to have informed something in the repository, that thing is removed and redone from allowed sources, and the commit says why. The survey's working notes, which recorded Symbolica details from its website, were deleted on 15 October 2026 without being committed; that is the model.
8. **Revisiting.** If Symbolica's licence changes, a new record may supersede this one. Until then the rule holds even where a particular use might arguably be allowed.

## Alternatives considered

- **Use Symbolica freely as one engine among many, as before 15 September 2026:** the strongest alternative technically, since its author is a former FORM developer and its benchmarks would be informative. Rejected: the licence forbids exactly this for a Competing Product, permissions terminate on breach, and a GPL project cannot carry that risk into every fork.
- **Use only releases from before 15 September 2026:** possibly permitted, but the survey did not verify whether those releases keep their earlier terms, and a rule with a version boundary is harder to follow and to check than one without. Not chosen.
- **Take legal advice first and decide afterwards:** not excluded; it is Thomas's decision. The rule as recorded is the cautious reading, so legal advice could only relax it, never make the project unsafe in the meantime.
- **No written rule, only care:** cheaper, but the next maintainer, contributor or AI assistant would not know the boundary. [REQ-092](../requirements.md#req-092) asks that a newcomer can work without help; this record is that help.

## Consequences

- Easier: every design record can be traced to allowed sources, which is what the licence's independent-development carve-out needs.
- Harder: Tante cannot compare its speed or output with the engine closest to it in purpose, so [REQ-056](../requirements.md#req-056)'s factors are stated against FORM only, and correctness cross-checks ([REQ-071](../requirements.md#req-071)) use FORM, GiNaC, SymPy, FLINT and Tante's own implementation ([REQ-097](../requirements.md#req-097)).
- Harder: importing an external benchmark table needs a deliberate step to strip a column.
- Follow-up: when the reference corpus imports polybench ([prior-art.md §5](../prior-art.md#5-validation-corpus-catalogue), entry C1.4), the import notes that the Symbolica column was removed.
- Follow-up: the contributing guide of [REQ-080](../requirements.md#req-080) states this rule; `docs/development.md` (stage 6) repeats it in the list of allowed sources.
- Untested: whether a court would read "Competing Product" as covering Tante; this record does not depend on the answer.
