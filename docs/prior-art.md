# Tante: Prior Art
Version: 1.0 | 15 October 2026

What eight existing engines teach Tante. Status claims are "as of 15 October 2026" unless another date is given. This survey answers [OPEN-B](requirements.md#open-b) and is cited as evidence by [requirements.md](requirements.md) (written against v0.6). It was produced with the deep-research process: five research strands (FORM; GiNaC, SymEngine/SymPy and Symbolica; tensor systems; physics packages; validation corpus), each citing its sources, then synthesised.

## 1. Summary of findings

**FORM is not abandoned, so Tante has to be justified by its design, not by FORM's decline.** FORM is now developed at [github.com/form-dev/form](https://github.com/form-dev/form); `vermaseren/form` redirects there. It released **5.0.0 on 27 January 2026, 5.0.1 on 24 June 2026 and 5.0.2 on 18 September 2026** ([releases](https://github.com/form-dev/form/releases)). It had 187 commits in 2025 and 161 in 2026 up to mid-October ([commit search](https://api.github.com/search/commits?q=repo:form-dev/form+committer-date:2026-01-01..2026-12-31)). On the other hand, about three people do most of the work, and the FORM 5 paper says that since Vermaseren retired in 2018 FORM's future "increasingly depends on broader support and community involvement" ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 5). The case for a new engine ([REQ-004](requirements.md#req-004)) therefore rests on FORM's documented structural limits: a per-term size cap, a text preprocessor used for program logic, order-dependent `once` matches, $-variables that block parallelism, no library API, and documentation the developers themselves call incomplete. It does not rest on FORM being unmaintained.

**FORM is still the only open-licensed system with larger-than-memory expressions.** Its design is pointer-free terms, depth-first streaming of terms through each module, and a disk-based external merge sort as the only global operation ([arXiv:1203.6543](https://arxiv.org/abs/1203.6543)). I found no out-of-core capability documented for GiNaC, SymEngine, SymPy, Cadabra2, Redberry, xAct or FeynCalc. Out of FORM's own experience, three points carry over to Tante. FORM 5 deprecated MPI-based ParFORM, judging single-node threading "far superior" on modern hardware ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 4.3). FORM 5 routes polynomial arithmetic through FLINT, with speed-ups of 16× to 53× on multivariate rational-function workloads (sec. 3.3). And FORM 5 deprecated native Windows (sec. 4.1). These findings confirm [REQ-052](requirements.md#req-052) and [REQ-054](requirements.md#req-054), and they sharpen the performance target of [OPEN-F](requirements.md#open-f): FORM 5 with FLINT is a much harder baseline than FORM 4.

**The tensor systems agree on what users expect.** Cadabra2 and xAct both canonicalise monomials with the Butler–Portugal double-coset algorithm (Cadabra reuses xAct's xPerm C code). Both treat multi-term symmetries such as Bianchi identities as a separate layer ([arXiv:0803.0862](https://arxiv.org/abs/0803.0862); [Cadabra manual](https://cadabra.science/manual/canonicalise.html)). Redberry shows the alternative of graph-isomorphism equality tests, and also a warning: a strong design that became hard to reach within about a decade ([arXiv:1302.1219](https://arxiv.org/abs/1302.1219)). FeynCalc is the model for an executable, literature-validated example corpus: about 172 scripts hard-fail when a result disagrees with a textbook. It is also a model for an honest γ₅ contract. Its own authors send heavy work to FORM ([arXiv:2001.04407](https://arxiv.org/abs/2001.04407); [arXiv:2312.14089](https://arxiv.org/abs/2312.14089)).

**Symbolica is described here only at the level of public positioning.** Its Source-Available License 1.0 of 15 September 2026 forbids using it, its documentation or anything learned from it as a blueprint, oracle or benchmark for a competing product, including free software ([License.md](https://github.com/benruijl/symbolica/blob/main/License.md)). This survey leaves out its internals on purpose (section 2.4).

**A validation corpus can be assembled almost entirely from redistributable or restatable sources** (section 5). These include FORM's GPL-3.0 test suite of about 560 default tests, the Fateman and Monagan–Pearce polynomial benchmarks, MIT-licensed polybench, Wester's problems restated with attribution, the colour-factor tables of van Ritbergen, Schellekens and Vermaseren, FeynCalc's textbook-checked examples, and the five-loop β function from three independent groups.

### Corrections to the survey brief's citations

| Brief's citation | Correction | Source |
|---|---|---|
| arXiv:0809.1145 cited as the TFORM paper | Not the TFORM paper. TFORM is Tentyukov & Vermaseren, **hep-ph/0702279**, CPC 181 (2010) 1419. What 0809.1145 actually is was not checked. | [hep-ph/0702279](https://arxiv.org/abs/hep-ph/0702279) |
| ParFORM reference | **cs/0407066** ("We have now a parallel version of FORM running on Cluster- and SMP-architectures") | [cs/0407066](https://arxiv.org/abs/cs/0407066) |
| arXiv:1006.4512 | **"FORM facts"** (Vermaseren, 2010), not a version paper | [arXiv:1006.4512](https://arxiv.org/abs/1006.4512) |
| arXiv:1606.08659 | **Baikov, Chetyrkin & Kühn**, "Five-Loop Running of the QCD coupling constant", PRL 118 (2017) 082002 | [arXiv:1606.08659](https://arxiv.org/abs/1606.08659) |
| arXiv:2512.19858 as a "LoopScalla" paper (in a garbled FORM 5 reference list) | It is Shtabovenko, "FeynCalc 10.2 and FeynHelpers 2". It announces LoopScalla in sec. 8. | [arXiv:2512.19858](https://arxiv.org/abs/2512.19858) |
| GiNaC uses "hash-consing" | Not confirmed. GiNaC shares data through reference-counted copy-on-write handles and uses hashes for comparison. No global unique-object table is described. | [GiNaC tutorial App. A.1](https://www.ginac.de/tutorial/) |
| FORM "open source since 2000" (2025 tutorial slide) | FORM 3 (2000) was free binaries. The GPL-3.0 licence arrived with version 4 (open-sourcing announced 2010, paper 2012). | [math-ph/0010025](https://arxiv.org/abs/math-ph/0010025) sec. 5; [arXiv:1203.6543](https://arxiv.org/abs/1203.6543) |
| MBtools and Reduze 2 use GiNaC | **Not verified** | — |

## 2. The systems one by one

### 2.1 FORM: streaming terms and a disk sort, now with FLINT

**Status.** FORM's release history is v4.0 (code from 2012), 4.1 (2013), 4.2.0 (2017), 4.2.1 (2019), 4.3.0 (2022), 4.3.1 (11 April 2023), then the three 5.0.x releases of 2026 ([GitHub releases API](https://api.github.com/repos/form-dev/form/releases)). By commit count the main contributors are Takahiro Ueda (405 commits), Jos Vermaseren (254), Joshua Davies (223), Ben Ruijl (58) and Coenraad Marinissen (32) ([contributors API](https://api.github.com/repos/form-dev/form/contributors)). The issue tracker had 144 open and 453 closed issues on 15 October 2026. In the preceding year, 41 issues were opened and 132 closed, so the backlog is shrinking ([issue search](https://api.github.com/search/issues?q=repo:form-dev/form+is:issue+is:open)). FORM has no programme funding. The 2026 paper acknowledges only personal grants: STFC ST/X000699/1 and JSPS KAKENHI JP22K03604 and JP24K07055 ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982)). Developer workshops have been held every year since 2023: Madrid 2023, Zurich 2024, Liverpool 2025, Nikhef 22–26 June 2026, and CERN 5–9 July 2027 (announced) ([Nikhef 2026](https://indico.nikhef.nl/event/7533/); [CERN 2027](https://indico.cern.ch/event/1712701/)). I found no governance document or named succession plan.

**Representation and canonical form.** A term is a flat array of integer words with no pointers. It holds a length, then subterms (each with a type code and a length), then a coefficient: numerator, denominator and a signed length ([sources/ftypes.h](https://github.com/form-dev/form/blob/master/sources/ftypes.h); [sources/sort.c](https://github.com/form-dev/form/blob/master/sources/sort.c)). The layout was reconstructed from source; I found no single authoritative document of it. `WORD` is `int32_t` on 64-bit builds ([form3.h](https://github.com/form-dev/form/blob/master/sources/form3.h)). Every statement is "local", meaning it acts on one term at a time. Sorting is the non-local operation that brings an expression to canonical form at the end of each module. In the developers' words, "The art of making good Form programs is often defined by how to construct inherently non-local operations by a combination of local operations and non-local sort instructions" ([arXiv:1203.6543](https://arxiv.org/abs/1203.6543), sec. 2). Terms contain no pointers and share no common subexpressions, so "there is no need for reference counts" (same source).

**Out-of-core.** An expression is "a sequence of terms stored sequentially on disk unless the expression is small enough to fit inside a cache buffer" ([arXiv:1203.6543](https://arxiv.org/abs/1203.6543)). The sort runs in four stages ([FORM manual](https://form-dev.github.io/form-docs/stable/manual/), "The setup"):

1. Terms are pointer-sorted in a small buffer.
2. The sorted patches are merged in a large buffer.
3. The large buffer is written as patches to a sort file.
4. The sort file is merged FilePatches (default 256) at a time, in further passes if needed.

Sort files are prefix-compressed against the previous term. FORM 5 adds Zstandard compression, giving up to about 8% runtime gain and about 6% smaller files ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 3.5). At this scale, in 2010 an MZV data-mine job generated **28,710,904,088,430 terms**, and one module produced 2.56 × 10⁹ output terms in 61.6 GB ([arXiv:1006.4512](https://arxiv.org/abs/1006.4512), sec. 2). FORM 4.2 described its target as "billions of terms that take up several terabytes of disk space" ([arXiv:1707.06453](https://arxiv.org/abs/1707.06453)).

The costs of this design are documented too. MaxTermSize caps one term at 40,000 words by default on 64-bit builds, and MaxNumberSize must stay below half of it. Buffer sizes are tuned in a setup file ([manual](https://form-dev.github.io/form-docs/stable/manual/)). In the FORM 5 paper's words, "Form makes large memory allocations that may only be used for a small percentage of its runtime" ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 3.4). Before FORM 5, concurrent jobs sharing a temporary directory crashed (sec. 2.3). FORM 5 deprecated the checkpoint mechanism as "very likely to contain bugs" (sec. 4.4; [issue #626](https://github.com/form-dev/form/issues/626)).

**Parallelism.** TFORM (POSIX threads) uses a master/worker model. The master hands out terms in buckets, 500 by default, with twice as many buckets as workers. Workers sort their own output, and the master merges the sorted streams. Reported speed-ups were about **1.7× on 2 cores and slightly above 3× on 4 cores** ([hep-ph/0702279](https://arxiv.org/abs/hep-ph/0702279)). The same paper names the bottlenecks: "In its very last stage it is a single processor that will have to do the final merge", and a single expensive term cannot be split across workers. Distributing whole indexed brackets helped but "still leaves the bottlenecks at the end of the sorting" ([arXiv:1006.4512](https://arxiv.org/abs/1006.4512)). $-variables carry state between terms, so a module that uses one runs sequentially unless a `ModuleOption` declares how it is reduced (sum, maximum, minimum or local). FORM 5 now warns when this happens ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 2.7). ParFORM (MPI, [cs/0407066](https://arxiv.org/abs/cs/0407066)) is deprecated: "Now that single systems may have several hundred CPU cores and multiple TB of memory, the parallelisation methods of TForm on single systems are far superior" ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 4.3; [issue #625](https://github.com/form-dev/form/issues/625)). I found no explicit guarantee that TFORM output is identical to FORM output, and no scaling curves beyond 8 cores apart from FORM 5's 24-worker FLINT table.

**Pattern matching.** The basic statement is `id lhs = rhs;`, with options `multi`, `many`, `once`, `only`, `select`, `disorder` and `all` (the last added in 4.2 for Forcer). Wildcards are written `x?` and can be restricted to sets or by conditions. Argument-field wildcards are written `?a`. Rules are iterated with `repeat; … endrepeat;` ([manual](https://form-dev.github.io/form-docs/stable/manual/); [arXiv:1707.06453](https://arxiv.org/abs/1707.06453)). The manual itself warns that with `once`, the match taken "may depend on the order of declaration of variables. It could also be installation dependent". It also warns that several argument-field wildcards make matching slow, and that argument-field wildcards are not allowed in symmetric functions. Transform statements were added because argument-by-argument `repeat id` "always [involves] a superfluous amount of pattern matching" ([arXiv:1006.4512](https://arxiv.org/abs/1006.4512)). Polynomial gcd and factorisation arrived in FORM 4.0 ([arXiv:1203.6543](https://arxiv.org/abs/1203.6543)). FORM 5 delegates PolyRatFun, FactArg, `gcd_` and related operations to FLINT 3.2 or newer by default. Whole-expression `Factorize` and `Modulus` mode do not use FLINT yet, and the overall sign of `gcd_` may differ from earlier versions ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 3.3).

**Test suite and CI.** `check/` holds a Ruby test/unit runner, `check.rb`. Each test is a fold containing a FORM program and Ruby assertions that regex-match the printed output, for example `assert result("F") =~ expr("1 + 2*x + x^2")` ([check/README.md](https://github.com/form-dev/form/blob/master/check/README.md)). Counting fold markers on 15 October 2026 gave about **560 default tests**: examples.frm 128, features.frm 117, fixes.frm 250, user.frm 5, polynomial.frm 39, coverage.frm 13 and diagrams.frm 8. `check/extra` adds 27 heavier tests: Forcer 5, Mincer 4, mzv-dm 10, z-fmft 3, and one each for checkpoint, color, multithreaded and series ([check/](https://github.com/form-dev/form/tree/master/check)). Regression tests record their provenance, for example "Bugs reported 2004-04-06 … Fixed 2005-09-27" ([fixes.frm](https://github.com/form-dev/form/blob/master/check/fixes.frm)). CI runs on Ubuntu 24.04. The test matrix covers form and tform, default and extra tests, with and without FLINT, plus parform. There are also Valgrind runs, Coveralls coverage and a 32-bit container ([test.yml](https://github.com/form-dev/form/blob/master/.github/workflows/test.yml)). Release builds cover Linux x86-64 and ARM64, macOS Intel and ARM, and Windows through MSYS2 ([deploy.yml](https://github.com/form-dev/form/blob/master/.github/workflows/deploy.yml)). The developers write that the suite needs "further work … to have sufficient coverage" ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 1). The suite was not run for this survey.

**Documentation.** FORM's documentation consists of the following:

- a single-page reference manual of about 96,000 words ([stable manual](https://form-dev.github.io/form-docs/stable/manual/));
- the "FORM for Pedestrians" tutorial, dated October 2000 and still marked draft ([tutorial](https://www.nikhef.nl/~form/maindir/documentation/tutorial/online/online.html));
- a doxygen developer reference ([doc/devref](https://github.com/form-dev/form/tree/master/doc/devref));
- version papers from 2000, 2010, 2012, 2017 and 2026.

The changes from 4.2.1 to 4.3.1 "were never described in a paper" ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 2.1). In 2010 FORM was not yet open source partly because that required "a good documentation of the inner workings" ([arXiv:1006.4512](https://arxiv.org/abs/1006.4512)). The manual names a silent misparse, a missing semicolon after a declaration, as "the most common error for which FORM cannot give a direct error message" (sec. 2.13).

In a 2023 talk to the FORM workshop, Ruijl, a former core developer, criticised FORM's programming model: "Counterintuitive control flow", "Text-based preprocessor is used for logic", "Hard to act at the expression level due to implicit loop over terms", "Term length limitations", "Not possible to use as a library" ([slides, Madrid, 13 April 2023](https://indico.ift.uam-csic.es/event/17/contributions/207/attachments/144/217/symbolica.pdf)). Only his statements about FORM are used here. The same slides introduce Symbolica, and nothing from them about Symbolica is carried into this report (section 2.4).

**Licence.** FORM 1 was free, FORM 2 commercial, and FORM 3 (2000) free binaries ([math-ph/0010025](https://arxiv.org/abs/math-ph/0010025)). From version 4 it is "an open source program under the GNU General Public License (GPL) version 3" ([arXiv:1203.6543](https://arxiv.org/abs/1203.6543)). Source headers say "version 3 of the License, or (at your option) any later version" ([sort.c](https://github.com/form-dev/form/blob/master/sources/sort.c)).

**Physics record.** FORM, or the Mincer package written in FORM, was used for:

- the 3-loop QCD β function ([hep-ph/9302208](https://arxiv.org/abs/hep-ph/9302208));
- the 4-loop QCD β function ([hep-ph/9701390](https://arxiv.org/abs/hep-ph/9701390));
- the 3-loop splitting functions ([hep-ph/0403192](https://arxiv.org/abs/hep-ph/0403192), [hep-ph/0404111](https://arxiv.org/abs/hep-ph/0404111));
- both five-loop β-function calculations ([arXiv:1606.08659](https://arxiv.org/abs/1606.08659); [arXiv:1701.01404](https://arxiv.org/abs/1701.01404) via Forcer, [arXiv:1704.06650](https://arxiv.org/abs/1704.06650)).

### 2.2 GiNaC: a C++ library with an order that depends on declarations

**Status.** GiNaC is active. Version 1.8.10 (11 February 2026) added class `Gt` for elliptic multiple polylogarithms and requires C++14. Version 1.8.9 (15 May 2025) fixed GCC 15 builds ([GiNaC News](https://www.ginac.de/News.html)). It is a C++ library with the `ginsh` shell, and pyGiNaC is listed as related software ([ginac.de](https://www.ginac.de/)). GiNaC was designed for multivariate polynomials and special functions in QFT loop calculations, and it deliberately uses C++ as both implementation and user language ([cs/0004015](https://arxiv.org/abs/cs/0004015)).

**Representation and canonical form.** Expressions are "handles to other more fundamental objects" with "an intrusive reference-counting mechanism" and "copy-on-write semantics". Sums and products share the `expairseq` structure: (expression, numeric coefficient) pairs plus an overall coefficient ([tutorial §4.1, App. A](https://www.ginac.de/tutorial/)). The canonical order is "deterministic, but not lexicographical", and it "almost always depends on the number and order of the symbols you define". Symbols carry hidden serial numbers, and comparison uses `calchash()` hash values ([tutorial §4.1.1, §6.5.4](https://www.ginac.de/tutorial/)). Automatic evaluation is limited to transformations "at most of complexity O(n log n)" ([§4.2](https://www.ginac.de/tutorial/)).

**Out-of-core and parallelism.** GiNaC's documentation describes neither. I found no thread-safety statement in the tutorial sections I read; this is absence of evidence only.

**Pattern matching.** Matching is "syntactic, not algebraic". More than one wildcard as a term of a sum or factor of a product "will lead to unpredictable or ambiguous results", and the `algebraic` option does not find x+y within x+y+z ([tutorial §5.4](https://www.ginac.de/tutorial/)). Factorisation works over the rationals only and "cannot handle algebraic extensions" ([§5.7–5.8](https://www.ginac.de/tutorial/)).

**Documentation.** GiNaC has a thorough single-page tutorial of about 340,000 characters, with an internals appendix, plus a developer reference and a Mini-FAQ updated 20 February 2026 ([ginac.de](https://www.ginac.de/)).

**Licence.** GiNaC is distributed under "the GNU General Public License (GPL)" ([ginac.de](https://www.ginac.de/)). Whether that is GPL-2.0-or-later was **not verified** from its COPYING file. In physics, GiNaC hosts the reference implementation for numerical evaluation of multiple polylogarithms ([hep-ph/0410259](https://arxiv.org/abs/hep-ph/0410259)) and now elliptic ones ([arXiv:2602.09956](https://arxiv.org/abs/2602.09956)). HepLib (GPL-3.0, C++) is built on GiNaC and uses FORM for Dirac and colour algebra ([arXiv:2103.08507](https://arxiv.org/abs/2103.08507)).

### 2.3 SymEngine and SymPy: hashed trees in memory

**Status.** SymEngine v0.15.0 was released on 26 August 2026. symengine.py was last released as v0.14.1 on 20 April 2025. SymPy was last released as 1.14.0 on 27 April 2025, with commits continuing daily ([symengine](https://github.com/symengine/symengine); [sympy](https://github.com/sympy/sympy); GitHub API, 15 October 2026). SymEngine is C++ with C, Python, Ruby, Julia and Haskell wrappers, and it can optionally use FLINT, MPFR, MPC and ARB ([README](https://github.com/symengine/symengine)).

**Representation.** SymEngine uses reference-counted pointer (RCP) trees with both hashed containers (`std::unordered_map`) and ordered containers (`std::map` with `RCPBasicKeyLess`) ([dict.h](https://symengine.org/symengine/dict_8h.html)). Hash-ordered sums imply that print order depends on hash values. I found no statement on whether those hashes are stable across platforms. The SymPy paper ([doi:10.7717/peerj-cs.103](https://doi.org/10.7717/peerj-cs.103)) could not be retrieved (HTTP 403), so SymPy internals are **not covered from a primary source**.

**Out-of-core, parallelism, pattern matching.** I found no out-of-core support for either project. SymEngine lists OpenMP as an optional dependency, but its scope is undocumented ([README](https://github.com/symengine/symengine)). Pattern matching (SymPy `Wild`, `match`, `replace`) was not sourced.

**Documentation and licence.** SymEngine's documentation is mostly Doxygen API reference ([symengine.org](https://symengine.org/symengine/namespaceSymEngine.html)). SymEngine is MIT ("All files are licensed under MIT license") ([README](https://github.com/symengine/symengine)). SymPy is BSD-3-Clause by common knowledge, but the GitHub API reports NOASSERTION, so this is **not verified**.

### 2.4 Symbolica: positioning and licence only, by deliberate choice

**Why this section is short.** Symbolica's Source-Available License 1.0 took effect on 15 September 2026 for the v3.0 series ([License.md history](https://github.com/benruijl/symbolica/commits/main/License.md)). It defines a "Competing Product" as software, *including free or open-source software*, whose computer-algebra functionality is "substantially similar to or substitutable for Symbolica". Section 4(4) forbids using the Software, its source, binaries or "Derived Information" "as a reference implementation, behavioral oracle, development blueprint, or other development input" for a Competing Product, "including to select, design, implement, … test, validate, benchmark, document … any computer-algebra … algorithm, architecture, data structure". Section 4(5) also forbids creating "any plan, roadmap, gap analysis, specification, … test suite, benchmark, output comparison" informed by those materials. "Software" includes the repository's documentation, comments and examples (§1). Section 5 forbids using AI systems to "explain, summarize, or otherwise recover or infer implementation-specific information" from it ([License.md](https://github.com/benruijl/symbolica/blob/main/License.md)).

A GPL-3.0 successor to FORM for physicists is very plausibly a Competing Product. This survey therefore **deliberately omits** Symbolica's internal representation, algorithms, pattern-matching features, streaming or out-of-core design, and benchmark numbers, and derives no recommendation from any of them. The working notes behind this survey recorded some such details from Symbolica's public website; they were not carried forward and are not part of the repository. This is a reading of the licence text, not legal advice. The licence carves out "independent development that does not use the Source Materials, Binary Materials, or Derived Information" (§4).

**Public positioning.** Symbolica is written by **Ben Ruijl**, formerly a core FORM developer (58 FORM commits, [contributors API](https://api.github.com/repos/form-dev/form/contributors); co-author of [FORM 4.2](https://arxiv.org/abs/1707.06453) and [Forcer](https://arxiv.org/abs/1704.06650)). He presented it at the 2023 FORM workshop as a "what if we started from scratch" project ([slides](https://indico.ift.uam-csic.es/event/17/contributions/207/attachments/144/217/symbolica.pdf)). It is a Rust library with a Python package ([GitHub](https://github.com/benruijl/symbolica)). Version 1.0 was announced on 23 November 2025. Number types and graph canonisation were then split into separate MIT-licensed crates, Numerica and Graphica, while Symbolica itself stayed source-available ([1.0 post](https://symbolica.io/posts/stable_release/index.html)). Symbolica is actively maintained: v2.1.0 (21 June 2026), v2.2.0 (23 July 2026) and v3.0.1 (6 October 2026), with a commit on 15 October 2026 and 994 stars ([releases](https://github.com/benruijl/symbolica/releases)). In 2024 the developer workshop in Zurich was billed as a joint "FORM and Symbolica developers meeting" ([Indico](https://indico.cern.ch/event/1386443/)).

**Licence restrictions.** The licence permits inspecting the source, building it internally, making private modifications and linking unmodified copies. It forbids distributing modified versions, using it as an oracle or blueprint for a Competing Product (including through a "clean-room team, automated tool, artificial-intelligence system"), and circumventing "core limits, instance limits" (§4(7)). Permissions terminate on breach (§7). Without a paid runtime licence, use is limited to **one core and one instance** for academic or non-commercial work. Academic group licences cost €1,500 to €4,000 per year ([symbolica.io/license](https://symbolica.io/license/)). I did **not verify** whether releases before 15 September 2026 (≤2.2.0) remain under their earlier terms. The licence says it "applies separately to each version".

### 2.5 Cadabra2: TeX input, xPerm canonicalisation and `meld`

**Status.** Cadabra2 is active. Version 2.6.0 was released on 14 October 2026, moving to GTK4 and adding Debian packages and pip install. Earlier releases were 2.5.14 (31 July 2025) and 2.5.10 (24 December 2024), the latter with `substitute` speed-ups ([cadabra.science](https://cadabra.science/)). It builds on Linux, macOS, Free/OpenBSD and Windows (MSYS2 UCRT64), and has about 4,115 commits ([GitHub](https://github.com/kpeeters/cadabra2)).

**Representation and canonical form.** Input is "a subset of TeX". Expressions are trees, and properties attach to symbols through a multiple-inheritance property system ([cs/0608005](https://arxiv.org/abs/cs/0608005)). Index features include multiple index types, upper and lower positions, flat and curved indices, and dimension-dependent identities such as Schouten ([cadabra.science](https://cadabra.science/)). `canonicalise` uses mono-term symmetries (`Symmetric`, `AntiSymmetric`, `TableauSymmetry`) through **xPerm's C code**, which the README credits to J. M. Martín-García ([README](https://github.com/kpeeters/cadabra2)). It does not handle multi-term symmetries, and `rename_dummies` is often needed afterwards ([manual: canonicalise](https://cadabra.science/manual/canonicalise.html)). `meld` combines terms using mono- and multi-term symmetries by removing linear dependencies rather than computing a canonical form. Its documented example collapses a Riemann–Riemann pair into one term with coefficient 3/2 ([manual: meld](https://cadabra.science/manual/meld.html)). The algorithm behind `meld` was not traced to a paper.

**Out-of-core and parallelism.** None documented.

**Pattern matching.** Substitution is index-aware and works up to dummy renaming ([manual](https://cadabra.science/manual/canonicalise.html)).

**Documentation and licence.** Cadabra2 has a manual, a JOSS paper ([doi:10.21105/joss.01118](https://joss.theoj.org/papers/10.21105/joss.01118)), worked GR and QFT examples ([arXiv:2210.00005](https://arxiv.org/abs/2210.00005); [arXiv:2210.00007](https://arxiv.org/abs/2210.00007)), and Jupyter and notebook front ends. It is GPL-3.0 ([GitHub](https://github.com/kpeeters/cadabra2)).

### 2.6 Redberry: graph matching instead of canonical forms, and a warning about link rot

**Status.** Redberry is effectively unmaintained. The GitHub repositories were last updated in 2020 (`redberry`) and 2013 (`core`, `physics`, `groovy`) ([github.com/redberry-cas](https://github.com/redberry-cas)). redberry.cc refused connections on 15 October 2026. Maven artifacts remain ([mvnrepository](https://mvnrepository.com/artifact/cc.redberry/physics/versions)). It is Java 7 with a Groovy front end that pulls dependencies with `@Grab` ([arXiv:1302.1219](https://arxiv.org/abs/1302.1219)).

**Representation and canonical form.** Expressions are immutable `Tensor` objects. Each index is a 32-bit integer encoding upper or lower position, type and name. Symmetries are permutation groups with strong generating sets. Redberry does **not** canonicalise. It decides equality by searching for a signed mapping between free indices, which is a graph-isomorphism-style problem, and its "standard form" orders terms by hash codes that are randomised per run unless seeded ([arXiv:1302.1219](https://arxiv.org/abs/1302.1219)). The authors argue that canonicalisation cannot compare expressions up to free-index relabelling. They also call it "NP-hard", which is their claim and not a standard result. Multi-term symmetries (Bianchi) were future work in that paper.

**Out-of-core and parallelism.** None documented.

**Pattern matching.** Substitutions match through index mappings with automatic resolution of dummy clashes. `&` applies substitutions in sequence and `|` simultaneously. Transformations such as `Expand`, `EliminateMetrics` and `DiracTrace` are composable objects ([arXiv:1302.1219](https://arxiv.org/abs/1302.1219)).

**Performance.** Self-reported benchmarks show about 26× to 41× speed-ups over xAct 1.1.0 and Cadabra 1.29. Redberry itself "failed to simplify large expressions in a reasonable time" when symmetries were present (same source).

**Licence.** The paper states GPL v3. GitHub shows GPL-3.0 for most repositories but GPL-2.0 for `core`; this conflict is **unresolved**.

### 2.7 xAct: Butler–Portugal canonicalisation and invariant databases, inside Mathematica

**Status.** xAct is active. v1.3.0 (29 December 2025) added eight packages, including TInvar, which canonicalises Riemann expressions with free indices, and xBrauer. The site lists 27 contributed packages and more than 1,000 papers using xAct as of 22 November 2023 ([xact.es](http://www.xact.es/)). It requires proprietary Mathematica. The minimum version, and whether the free Wolfram Engine suffices, were **not verified**.

**Representation and canonical form.** xAct is layered: xCore, xPerm (permutation groups), xTensor (abstract tensors) and xCoba (components) ([xact.es](http://www.xact.es/)). xPerm implements Butler–Portugal canonicalisation, with free indices treated as a coset problem ([math-ph/0107031](https://arxiv.org/abs/math-ph/0107031)) and dummy indices as a double-coset problem ([math-ph/0107032](https://arxiv.org/abs/math-ph/0107032)). It has a C core and was "effectively polynomial" in its author's tests, handling about 100 indices in seconds ([arXiv:0803.0862](https://arxiv.org/abs/0803.0862)). Niehoff showed Butler–Portugal can still blow up as O(n!) on highly symmetric inputs, and gave a polynomial-time treatment of totally (anti)symmetric subsets ([arXiv:1702.08114](https://arxiv.org/abs/1702.08114)). Multi-term identities are handled by the Invar database: cyclic and Bianchi identities, derivative commutation, dimension-dependent identities and duals, covering more than 6 × 10⁵ relations ([arXiv:0802.1274](https://arxiv.org/abs/0802.1274)). xPert computes metric perturbations to arbitrary order, reaching order 10 in under an hour ([arXiv:0807.0824](https://arxiv.org/abs/0807.0824)).

**Out-of-core, parallelism, pattern matching.** No out-of-core or parallel features were found. xTensor's rule mechanism was not sourced.

**Documentation and licence.** xAct has per-package Mathematica notebooks ([xPermDoc](https://xact.es/Documentation/HTML/xPermDoc.nb.html)). The licence is stated as "GPL 2002–2026"; the exact version was **not verified**.

### 2.8 FeynCalc and related physics packages: semi-automatic Mathematica tools that hand heavy work to FORM

**FeynCalc status.** FeynCalc is maintained, mainly by Vladyslav Shtabovenko. Recent releases are 10.0.0 (21 December 2023), 10.2.0 (22 December 2025) and 10.2.1 (11 June 2026). The last commit was on 9 September 2026, fixing a bug in `SUNSimplify` ([releases](https://github.com/FeynCalc/feyncalc/releases); [commits](https://github.com/FeynCalc/feyncalc/commits/master)). It supports Mathematica 10 onward ([arXiv:2512.19858](https://arxiv.org/abs/2512.19858)). Between 2001 and 2014 it received mostly bug fixes ([arXiv:1601.01167](https://arxiv.org/abs/1601.01167)).

**Representation.** FeynCalc consists of typed wrappers on Mathematica expressions. Objects live in 4, D or D−4 dimensions. Mixed contractions follow BMHV rules, and other dimension specifications are not supported ([FAQ](https://github.com/FeynCalc/feyncalc/blob/master/FeynCalc/Documentation/Markdown/Extra/FrequentlyAskedQuestions.md)). The γ₅ schemes are NDR (the default), NDR-Discard, BMHV and Larin. In NDR, traces that are ambiguous at O(D−4) are **returned unevaluated** rather than silently resolved. The stated contract is that FeynCalc guarantees algebraic consistency within the chosen scheme, and that restoring Ward identities is "the duty of the user" ([arXiv:2001.04407](https://arxiv.org/abs/2001.04407), sec. 4.2). Mathematica does not type-check operators, so FeynCalc "happily accepts obviously incorrect inputs". An opt-in `FCCheckSyntax` was added (same source, sec. 4). FeynCalc does not support spinor-helicity ([arXiv:2312.14089](https://arxiv.org/abs/2312.14089), sec. 6.6).

**Scale and parallelism.** The developers write: "The performance is admittedly much worse as compared to codes that use faster symbolic manipulation systems, notably FORM". For hundreds of thousands to millions of terms, the "only sensible way" is to offload heavy steps to FORM ([arXiv:2001.04407](https://arxiv.org/abs/2001.04407), sec. 1). Version 10.2 added parallel subkernels. On 21 two-loop gg→H diagrams with 8 kernels, `DiracSimplify` went from 26.9 s to 7.9 s ([arXiv:2512.19858](https://arxiv.org/abs/2512.19858), sec. 6.1).

**Tests and documentation.** A 2016 paper reports "over 3000 tests" ([arXiv:1601.01167](https://arxiv.org/abs/1601.01167)). A regex count of the repository on 15 October 2026 found about **14,500 MUnit input/output entries in 283 files**, run locally from shell scripts. No `.github/workflows` was found ([Tests/](https://github.com/FeynCalc/feyncalc/tree/master/Tests)). About 172 example scripts compare against literature values with `FCCompareResults` and exit with an error on mismatch. One example checks the anomalous magnetic moment `AlphaFS/(2Pi)` against Schwinger 1948 ([El-GaEl.m](https://github.com/FeynCalc/feyncalc/blob/master/FeynCalc/Examples/QED/OneLoop/Mathematica/El-GaEl.m)). Version 10 moved the documentation to about 746 Markdown pages, rebuilt automatically, with conceptual pages on γ₅, FeynArts signs and Schouten ambiguities. The authors cite the earlier "lack of a proper manual" as the reason ([arXiv:2312.14089](https://arxiv.org/abs/2312.14089), sec. 6.7).

**Licence.** GPL-3 ([feyncalc.github.io](https://feyncalc.github.io/)).

**Related packages.** FeynArts 3.12 (10 August 2026) and FormCalc 9.10 (11 October 2024) are LGPL. FormCalc does its algebra in FORM ([feynarts.de](https://feynarts.de/); [formcalc](https://feynarts.de/formcalc/)). Package-X was closed-source freeware with encrypted source, and "the development of Package-X has officially been abandoned" ([arXiv:2512.19858](https://arxiv.org/abs/2512.19858)). HEPMath (GPL-3) is also abandoned, even though it was open source ([arXiv:2312.14089](https://arxiv.org/abs/2312.14089)). FormTracer ([arXiv:1610.09331](https://arxiv.org/abs/1610.09331)) and HepLib ([arXiv:2103.08507](https://arxiv.org/abs/2103.08507)) both use FORM as their engine.

**The FORM colour package.** The FORM colour package of van Ritbergen, Schellekens and Vermaseren reduces colour factors to group invariants without fixing the gauge group. Its paper tabulates explicit SU(N), SO(N), Sp(N) and exceptional values ([hep-ph/9802376](https://arxiv.org/abs/hep-ph/9802376)). FORM's `check/extra/color.frm` derives a regression test from it ([color.frm](https://github.com/form-dev/form/blob/master/check/extra/color.frm)).

**Conventions.** Sign conventions differ between tools, and the differences are documented. In FeynCalc, `$LeviCivitaSign` = −1 gives Tr(γ^μγ^νγ^ργ^σγ₅) = −4i ε^{μνρσ}, and −I switches to "the FORM-convention" ([doc](https://github.com/FeynCalc/feyncalc/blob/master/FeynCalc/Documentation/Markdown/$LeviCivitaSign.md)). FeynArts vertex signs disagree with Peskin–Schroeder ([FeynArtsSigns](https://github.com/FeynCalc/feyncalc/blob/master/FeynCalc/Documentation/Markdown/Extra/FeynArtsSigns.md)). Romão and Silva tabulate the conventions of close to 40 textbooks ([arXiv:1209.6213](https://arxiv.org/abs/1209.6213)).

## 3. Cross-cutting comparison

| | FORM | GiNaC | SymEngine / SymPy | Symbolica | Cadabra2 | Redberry | xAct | FeynCalc |
|---|---|---|---|---|---|---|---|---|
| Latest release (as of 15 Oct 2026) | 5.0.2, 18 Sep 2026 | 1.8.10, 11 Feb 2026 | 0.15.0, 26 Aug 2026 / 1.14.0, 27 Apr 2025 | 3.0.1, 6 Oct 2026 | 2.6.0, 14 Oct 2026 | repos last updated 2013/2020; site down | 1.3.0, 29 Dec 2025 | 10.2.1, 11 Jun 2026 |
| Implementation | C, polynomial code in C++ ([arXiv:1203.6543](https://arxiv.org/abs/1203.6543)); uses FLINT, GMP, MPFR, zstd | C++ | C++ / Python | Rust, Python package | C++ core, Python front end | Java 7, Groovy | Mathematica, C core (xPerm) | Mathematica |
| Representation, canonical form | Pointer-free word arrays; canonical order set by the sort at the end of each module | Ref-counted copy-on-write `expairseq`; order depends on symbol creation, hash-assisted | RCP trees; hashed and ordered containers | *Deliberately omitted* | Trees with properties; Butler–Portugal via xPerm, plus `meld` | Immutable tensors; equality by index mapping; hash order randomised per run | Butler–Portugal (xPerm) plus Invar rule databases | Typed wrappers on Mathematica expressions |
| Larger than memory | Yes: disk merge sort, terabyte scale | None found | None found | *Deliberately omitted* | None found | None found | None found | No; delegates to FORM |
| Parallelism | TFORM threads (ParFORM deprecated); limited by $-variables and the final merge | Not documented | Optional OpenMP (scope undocumented) / not sourced | Free tier licence-limited to one core | None found | None found | None found | Mathematica subkernels (10.2) |
| Pattern matching | `id` with options, wildcards, `repeat`; `once` order-dependent | Syntactic; multi-wildcard sums ambiguous | Not sourced | *Deliberately omitted* | Index-aware, up to dummy renaming | Index mappings; `&` / `\|` composition | xTensor rules (not sourced) | Mathematica rules |
| Documentation | 96k-word manual; 2000 draft tutorial; papers; gaps acknowledged | Long tutorial plus reference | Mostly API reference | Not assessed | Manual, JOSS, notebooks | Paper; site unreachable | Notebooks per package | 746 Markdown pages, examples, tutorial |
| Tests / CI | ~560 + 27 tests; Linux CI, Valgrind, coverage | Not assessed | Not assessed | Not assessed | Not assessed | Not assessed | Not assessed | ~14.5k entries, no public CI |
| Licence | GPL-3.0-or-later | GPL (version unverified) | MIT / BSD-3 (unverified) | Source-Available 1.0, bars competing use | GPL-3.0 | GPL-3.0 (core shows GPL-2.0) | GPL (version unverified); needs Mathematica | GPL-3 |

## 4. Consequences for the requirements

### 4.1 A new and independent engine ([REQ-004](requirements.md#req-004))

The evidence **confirms** [REQ-004](requirements.md#req-004) but **revises its rationale**. FORM is actively maintained, so the argument for a new engine has to be about structure: the per-term cap, the setup-file tuning, the text preprocessor used for logic, the implicit loop over terms, order-dependent `once`, no library API, and incomplete documentation. FORM's own developers and manual record all of these (section 2.1). A rewrite should fix them rather than repeat them. FORM's small team is a second reason for an independent codebase: Tante should not depend on FORM's continued development. It is not a reason to expect FORM to disappear.

**Symbolica must not be consulted.** The note in [REQ-004](requirements.md#req-004) lists Symbolica among the engines whose design should "teach" the new engine, and [OPEN-E](requirements.md#open-e) calls Rust "Symbolica's choice". Both should be revised. Under §4(4)–(5) of Symbolica's licence, using it as a design input, benchmark or oracle for a competing GPL engine is forbidden. Tante's design records should therefore be traceable to non-Symbolica sources: FORM's papers and GPL source, GiNaC, SymEngine, SymPy, Cadabra2, xAct, Redberry and the algorithmic literature. An ADR should state that rule. FORM's own test suite, polybench and the published literature are safe sources. Any column of a benchmark result that reports Symbolica, such as in polybench, should be excluded.

[REQ-004](requirements.md#req-004) also forbids building on "another computer-algebra system's core". FORM 5 shows that delegating multivariate polynomial arithmetic to FLINT is now standard practice, and it gave 16× to 53× speed-ups ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982)). SymEngine also links FLINT ([README](https://github.com/symengine/symengine)). [REQ-004](requirements.md#req-004) should say explicitly whether an arithmetic library such as FLINT, GMP or MPFR counts as a "core". The recommended reading is that it does not. FLINT's own licence (commonly LGPL-3.0) was **not verified** in this survey.

Finally, the GPL-3.0 licences of FORM, Cadabra2 and FeynCalc allow reuse. xPerm's C canonicaliser is already reused by Cadabra2 under the GPL, which matters for stage 4. Whether the standalone `xperm.c` is GPL-3.0-compatible should be checked before reuse.

### 4.2 Multithreading ([REQ-052](requirements.md#req-052), [REQ-074](requirements.md#req-074))

The evidence **confirms** [REQ-052](requirements.md#req-052), and it supports leaving MPI out of the first release ([REQ-006](requirements.md#req-006)). The FORM team itself moved from ParFORM to TFORM ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 4.3). TFORM's history is a list of limits Tante should avoid:

- a single master doing the final merge;
- a single expensive term landing on one worker;
- the costs of signalling, solved by buckets;
- cross-term state ($-variables) that silently serialises a module unless its reduction is declared ([hep-ph/0702279](https://arxiv.org/abs/hep-ph/0702279); [arXiv:1006.4512](https://arxiv.org/abs/1006.4512)).

[REQ-074](requirements.md#req-074)'s determinism requirement is achievable only if three conditions hold. First, the canonical order must not depend on interning order or hash values. GiNaC's order "depends on the number and order of the symbols you define" ([tutorial](https://www.ginac.de/tutorial/)), and Redberry randomises hashes per run ([arXiv:1302.1219](https://arxiv.org/abs/1302.1219)). Second, every cross-term reduction must be declared commutative and associative, or it must run sequentially. Third, first-match semantics such as FORM's `once` must be specified rather than left "installation dependent" ([manual](https://form-dev.github.io/form-docs/stable/manual/)). Each of these could become an acceptance test.

As a minimum bar, TFORM achieved about 1.7× on 2 cores and just over 3× on 4 cores in 2007 ([hep-ph/0702279](https://arxiv.org/abs/hep-ph/0702279)). No modern scaling curves for FORM were found, so Tante's spike should measure TFORM 5.0.2 on the reference machine. A further positioning point: Symbolica's free academic tier is licence-limited to one core ([symbolica.io/license](https://symbolica.io/license/)), so a freely multithreaded GPL engine is distinctive.

### 4.3 Larger-than-memory expressions ([REQ-053](requirements.md#req-053), [REQ-054](requirements.md#req-054), [REQ-055](requirements.md#req-055))

The evidence **confirms** [REQ-054](requirements.md#req-054). Among the open-licensed systems surveyed, only FORM provides it, and FORM shows how. Terms are self-contained and pointer-free, so they can be written to disk and read back without fix-up. Every statement streams terms. Canonicalisation is an external merge sort with bounded fan-in, prefix compression and optional zstd ([arXiv:1203.6543](https://arxiv.org/abs/1203.6543); [arXiv:2601.19982](https://arxiv.org/abs/2601.19982)). [REQ-054](requirements.md#req-054)'s acceptance criterion, that no first-release interface assumes an expression is in memory as a whole, is the right one. It implies the first-release representation must already be relocatable, even if spilling to disk comes later.

The evidence also suggests **three additions**. First, a single term should be allowed to grow without a user-set cap. FORM's MaxTermSize and setup files are a long-standing complaint ([manual](https://form-dev.github.io/form-docs/stable/manual/); [Ruijl 2023](https://indico.ift.uam-csic.es/event/17/contributions/207/attachments/144/217/symbolica.pdf)). Second, memory should be released when it is not needed. FORM 5 had to add `#SortReallocate` for this ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 3.4). Third, temporary files must be safe when several jobs run concurrently (sec. 2.3), which matters on clusters ([REQ-051](requirements.md#req-051)).

[REQ-053](requirements.md#req-053)'s 10⁶ terms in memory is modest next to FORM's demonstrated 10⁹ to 10¹³ terms ([arXiv:1006.4512](https://arxiv.org/abs/1006.4512)). It is still reasonable for a first release, since GiNaC, SymEngine and the tensor systems show no out-of-core support at all.

[REQ-055](requirements.md#req-055) (checkpoint and resume) needs care. FORM 5 deprecated its own checkpoint mechanism as "very likely to contain bugs" ([issue #626](https://github.com/form-dev/form/issues/626)). Resume should therefore be built on the same immutable, stage-complete files as the sort, and tested by the `check/extra/checkpoint` style of test.

### 4.4 The validation corpus ([REQ-070](requirements.md#req-070), [REQ-071](requirements.md#req-071))

The evidence **confirms and extends** [REQ-070](requirements.md#req-070). Section 5 catalogues candidates by stage. Three findings bear on the requirement's wording.

First, snapshot tests on printed text are brittle. FORM's tests regex-match printed output ([check/README.md](https://github.com/form-dev/form/blob/master/check/README.md)), and FeynCalc's are exact input/output pairs. Tante should compare results in canonical form ([REQ-012](requirements.md#req-012)). It should add property checks such as h/f = g in the Fateman benchmark, and should store generator formulas plus invariants rather than large files.

Second, FeynCalc's pattern is worth adopting: executable examples that cite a textbook equation and hard-fail on mismatch ([El-GaEl.m](https://github.com/FeynCalc/feyncalc/blob/master/FeynCalc/Examples/QED/OneLoop/Mathematica/El-GaEl.m)). Run them in public CI, which FeynCalc lacks. This matches [REQ-091](requirements.md#req-091).

Third, licence status varies by source. FORM tests (GPL-3.0) and polybench (MIT) can be vendored with notices. Wester's problems, the Rubi suite and Forcer show no licence, so restate the mathematics with attribution and do not copy the files. Published results are facts and can be restated with citation.

Several [REQ-070](requirements.md#req-070) entries still lack a sourced expected result in this survey. These are the LO splitting functions, the e⁺e⁻→μ⁺μ⁻ squared amplitude (only the cross-section was located, and only from background knowledge) and the Unruh temperature (section 6).

### 4.5 The FORM version and the FORM corpus ([OPEN-D](requirements.md#open-d), [REQ-020](requirements.md#req-020), [REQ-021](requirements.md#req-021), [REQ-023](requirements.md#req-023))

The "current release" rule of [OPEN-D](requirements.md#open-d) currently means **FORM 5.0.2 (18 September 2026)**. FORM 5 changes behaviour in ways that matter for a reference. FLINT is on by default and can be switched per module. The overall sign of `gcd_` may differ from FORM 4. Floating-point coefficients and a built-in diagram generator are new ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982)). The compatibility specification should therefore record the build configuration, reported by `form -vv` (sec. 2.9), and whether FLINT is on, as well as the version number.

For the FORM corpus, `check/` provides about 560 default tests and 27 extra tests under GPL-3.0-or-later ([check/](https://github.com/form-dev/form/tree/master/check)). They can be vendored with attribution, and doing so does not conflict with [REQ-004](requirements.md#req-004), because tests are data and not engine code. `examples.frm` (128 tests, manual examples) and `fixes.frm` (250 regression tests with provenance) are the natural first targets for [REQ-021](requirements.md#req-021)'s fraction. The `extra` Mincer, Forcer, colour and MZV tests are the physics-grade tier.

Because assertions are regexes on FORM's printed term order, comparing results needs a canonical-form comparator. Character-level agreement ([OPEN-H](requirements.md#open-h), [REQ-023](requirements.md#req-023)) would tie Tante to FORM's term order. The evidence favours syntax-level FORM output plus canonical comparison. The Forcer repository showed no licence ([github.com/benruijl/forcer](https://github.com/benruijl/forcer)). Its inclusion beyond the copy in FORM's `check/extra` needs clarification.

### 4.6 Implementation language and platforms ([OPEN-E](requirements.md#open-e), [REQ-050](requirements.md#req-050))

The prior art does not settle the language, and the Symbolica licence means its choice cannot be used as an argument. The open-licensed evidence points to three facts:

- The engines that reach the largest scales or run fastest are written in C or C++ (FORM, GiNaC, SymEngine, Cadabra2's core). FLINT, the arithmetic backend now used by FORM and available to SymEngine, is a C library. So C++ has the lowest-friction path to FLINT and to the C++ library of [REQ-061](requirements.md#req-061). A Rust implementation would need FFI bindings to FLINT, whose maturity this survey did not check.
- Redberry shows the real risk: dependence on a toolchain and a hosting setup (Java 7, Groovy `@Grab`, a project website) that made a strong design hard to reach within about a decade ([arXiv:1302.1219](https://arxiv.org/abs/1302.1219); [redberry-cas](https://github.com/redberry-cas)).
- Package-X and HEPMath show that a single maintainer is the dominant abandonment risk, more than the language ([arXiv:2312.14089](https://arxiv.org/abs/2312.14089)).

For [REQ-050](requirements.md#req-050), the evidence **qualifies** the native-Windows requirement without overturning it. FORM 5 deprecated native Windows: "We are not aware of anyone making use of native Windows builds of Form for any serious computation… Form is mainly developed in Linux environments, making maintenance of Windows builds rather difficult" ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 4.1). FORM deprecated 32-bit builds for the same kind of reason (sec. 4.2). Cadabra2 does ship on Windows, through MSYS2 UCRT64 ([GitHub](https://github.com/kpeeters/cadabra2)). Native Windows is therefore achievable and would set Tante apart. The cost FORM names is real, and the out-of-core file handling of [REQ-054](requirements.md#req-054) is where it falls. Three-OS CI from the first commit is the mitigation, and the requirement should say 64-bit only.

### 4.7 Performance targets ([OPEN-F](requirements.md#open-f), [REQ-056](requirements.md#req-056))

[OPEN-F](requirements.md#open-f)'s "at most 3× slower than FORM" needs **revision by workload**. On term-streaming and pattern-rewriting workloads, FORM 5's core loop is little changed, so a factor against FORM is meaningful. On rational-function and gcd workloads, FORM 5 with FLINT is 16× to 53× faster than FORM without it ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), Table 1). A target against "FORM" is therefore ambiguous unless it fixes FORM 5.0.2 with FLINT on.

Absolute anchors from the literature exist for stage 1. FLINT computes the Fateman n=20 product in 0.53 s with its dense-array method, or 2.8 s with sparse methods and assembly, on a 2.2 GHz Opteron ([arXiv:1705.06134](https://arxiv.org/abs/1705.06134)). polybench (MIT) runs gcd and factorisation cases against FORM 5.0.1, FLINT, Singular, Fermat and others ([github.com/tueda/polybench](https://github.com/tueda/polybench/)). Excluding its Symbolica column, it is a ready harness for [REQ-056](requirements.md#req-056) measurements.

For parallel scaling, TFORM's 1.7× on 2 cores and about 3× on 4 cores ([hep-ph/0702279](https://arxiv.org/abs/hep-ph/0702279)) is the documented floor. The proposed 2% tracing overhead ([REQ-040](requirements.md#req-040)) has no prior-art comparator. The nearest is FORM's backtrace option, which costs up to 1% and is off by default ([arXiv:2601.19982](https://arxiv.org/abs/2601.19982), sec. 3.7).

### 4.8 Other requirements the evidence touches

[REQ-024](requirements.md#req-024) (errors in the user's terms) is supported by FORM's silent declaration misparse ([manual](https://form-dev.github.io/form-docs/stable/manual/) sec. 2.13) and FeynCalc's acceptance of `GA[mu] GA[nu] GA[mu]` written with Times instead of Dot ([arXiv:2001.04407](https://arxiv.org/abs/2001.04407)). [REQ-034](requirements.md#req-034) (explicit conventions) is supported by the FeynCalc/FORM Levi-Civita factor (−1 versus −I), the FeynArts vertex-sign differences and the 40-book survey (section 2.8). The FeynCalc γ₅ contract suggests a pattern for [OPEN-I](requirements.md#open-i): algebra consistent within a declared scheme, and ambiguous results left unevaluated. [REQ-017](requirements.md#req-017) should name where the index metadata enters: index type with its own dimension, variance, flat or curved, and dummy pool ([cadabra.science](https://cadabra.science/); [arXiv:1302.1219](https://arxiv.org/abs/1302.1219)). It should also name where a monomial canonicaliser and a separate multi-term layer enter. [REQ-090](requirements.md#req-090)–[REQ-092](requirements.md#req-092) are supported by the strongest documented complaints about FORM, which concern its programming model rather than missing reference text ([Ruijl 2023](https://indico.ift.uam-csic.es/event/17/contributions/207/attachments/144/217/symbolica.pdf); [CAPP 2025 slides](https://indico.desy.de/event/46419/contributions/175710/attachments/92426/132204/FORM_day1.pdf)). Conceptual guides are needed as well as a reference.

## 5. Validation-corpus catalogue

The "Verified" column uses three values. **Primary** means checked against the cited primary source during the survey. **Secondary** means checked only against a secondary source such as Wikipedia, and to be re-checked against the primary reference. **No** means quoted from background knowledge, and must be verified before the entry is accepted. IDs are proposed for citation from [REQ-070](requirements.md#req-070). Mathematical results are facts and may be restated with attribution; the licence column refers to files and code.

### Stage 1: numbers and polynomials

| ID | Entry | Source | Expected result | Establishes | Licence / redistributable | Verified |
|---|---|---|---|---|---|---|
| C1.1 | Fateman dense product: f=(1+x+y+z+t)²⁰, g=f+1, h=f·g, then h/f | Fateman, SIGSAM Bull. 37(1) 2003, [doi:10.1145/844076.844080](https://doi.org/10.1145/844076.844080); [Monagan–Pearce](https://cecm-www.dcr.sfu.ca/personal/monaganm/papers/Maple14.pdf) | f has 10,626 terms; h has 135,751 terms (83-bit coefficients); h/f = g exactly; h − f·g = 0 | Expansion, multi-precision coefficients, exact division; performance anchor | Formula only; nothing to redistribute | Primary (Monagan–Pearce) |
| C1.2 | Reduced Fateman (symbench S1): expand (x+y+z+1)⁷·((x+y+z+1)⁷+1) | [symbench](https://wiki.sagemath.org/symbench) | Term count not recorded; compute and cross-check with FLINT | Fast smoke test of C1.1 | Formula only | Primary (problem statement) |
| C1.3 | Monagan–Pearce sparse pair: (1+x+y+2z²+3t³+5u⁵)²⁵ · (1+u+t+2z²+3y³+5x⁵)²⁵ | [Maple14.pdf](https://cecm-www.dcr.sfu.ca/personal/monaganm/papers/Maple14.pdf) | Term count not recorded; compute and cross-check | Sparse multiplication | Formula only | Primary (problem statement) |
| C1.4 | polybench gcd and factorisation cases | [tueda/polybench](https://github.com/tueda/polybench/) | Nontrivial gcd and factors by construction | Multivariate gcd and factorisation correctness; timing against FORM 5.0.1, FLINT, Singular, Fermat | MIT, vendorable with notice; exclude Symbolica results | Primary (repo); generators not inspected |
| C1.5 | symbench R8: right Riemann sum of x² on [0,5], n=10⁴ | [symbench](https://wiki.sagemath.org/symbench) | 66676667/1600000 | Exact rational arithmetic at scale | Formula only | Primary (symbench page) |
| C1.6 | symbench R2: Hermite H₁₅(y) by recurrence, expanded | [symbench](https://wiki.sagemath.org/symbench) | Closed form of H₁₅ (from a standard table) | Repeated expansion and collection | Formula only | Problem: Primary; value: No |
| C1.7 | [REQ-012](requirements.md#req-012) example: (x+y)² − x² vs y·(2x+y) | [REQ-012](requirements.md#req-012) | Both print 2xy + y² identically | Canonical form | Own | n/a |
| C1.8 | Wester Algebra and ZeroEquivalence problems | [Wester review](https://math.unm.edu/~wester/cas_review.html) | Per problem | Breadth of correctness | **No licence**: restate with attribution, do not copy `.problems` files | Primary (page exists) |
| C1.9 | FORM `polynomial.frm` (39 tests) | [check/](https://github.com/form-dev/form/tree/master/check) | In file | gcd, factorisation, PolyRatFun semantics as FORM defines them | GPL-3.0-or-later, vendorable | Primary (count) |
| C1.10 | symbench R9 / W2: factor x²⁰ − π⁵y²⁰ and x²⁰ − (2^{1/3})⁵y²⁰ | [symbench](https://wiki.sagemath.org/symbench) | W2 needs algebraic extensions and was historically wrong in Sage | Factorisation; W2 is a trap and a scope boundary (GiNaC also lacks algebraic extensions) | Formula only | Primary (problem statement) |

### Stage 2: rational functions, differentiation, series

| ID | Entry | Source | Expected result | Establishes | Licence / redistributable | Verified |
|---|---|---|---|---|---|---|
| C2.1 | symbench R6: Σ_{i=0}^{99} ((x+sin i)/x + (x−sin i)/x), rationally simplified | [symbench](https://wiki.sagemath.org/symbench) | 200 | Rational normalisation with opaque function arguments | Formula only | Primary |
| C2.2 | symbench S3: expand (x^y + y^z + z^x)⁵⁰, then d/dx | [symbench](https://wiki.sagemath.org/symbench) | Not recorded | Differentiation of large sums with symbolic exponents | Formula only | Primary (problem) |
| C2.3 | symbench W5: coefficient of ψ''(r) in (1/r²)(2rψ' + r²ψ'') | [symbench](https://wiki.sagemath.org/symbench) | 1 | Collection by derivative objects | Formula only | Problem: Primary; value: own arithmetic |
| C2.4 | symbench S4: series of sin x · cos x to 1000 terms | [symbench](https://wiki.sagemath.org/symbench) | Coefficients of ½ sin 2x | Series truncation ([REQ-093](requirements.md#req-093)) at high order | Formula only | Problem: Primary |
| C2.5 | Γ(1+ε) to O(ε³) | [DLMF 5.7.3](https://dlmf.nist.gov/5.7.E3) | 1 − γε + (γ²/2 + π²/12)ε² + O(ε³) | ε-expansion with symbolic constants | Fact | No |
| C2.6 | Γ(1−ε)²Γ(1+ε)/Γ(1−2ε) to O(ε⁴); ₂F₁(aε,bε;1+cε;x) to O(ε³) | HypExp [hep-ph/0507094](https://arxiv.org/abs/hep-ph/0507094), [arXiv:0708.2443](https://arxiv.org/abs/0708.2443) | Regenerate with HypExp | Series with ζ values and polylogarithms | HypExp licence unchecked; regenerate rather than copy | No |
| C2.7 | FUEL standalone rational-function problems (e.g. rank-2 massless double box) | [arXiv:2304.13418](https://arxiv.org/abs/2304.13418) | In paper | Rational-function simplification at IBP scale | Paper; files unchecked | Primary (abstract only) |
| C2.8 | FORM 5 `mbox1l` multivariate IBP benchmark, (3,2,2,2) and (3,3,3,3) | [arXiv:2601.19982](https://arxiv.org/abs/2601.19982), Table 1 | FORM 5 with FLINT: 1.32 s and 11.2 s on 24 workers (Ryzen 9 7900X) | PolyRatFun performance against FORM 5 | Code location not confirmed | Primary (timings) |

### Stage 3: linear algebra

| ID | Entry | Source | Expected result | Establishes | Licence / redistributable | Verified |
|---|---|---|---|---|---|---|
| C3.1 | Symbolic Vandermonde determinant | Textbook ([Wikipedia](https://en.wikipedia.org/wiki/Vandermonde_matrix), not fetched) | ∏_{i<j}(x_j − x_i) | Determinant with factorised result | Fact | No |
| C3.2 | Cauchy determinant det[1/(x_i+y_j)] | Textbook | ∏_{i<j}(x_j−x_i)(y_j−y_i) / ∏_{i,j}(x_i+y_j) | Rational-function determinant | Fact | No |
| C3.3 | Hilbert matrix inverses | Textbook | Integer entries | Exact rational inversion | Fact | No |
| C3.4 | Wester MatrixTheory category | [Wester](https://math.unm.edu/~wester/cas_review.html) | Per problem | Rank, null space, singular systems | No licence: restate | Primary (page) |
| C3.5 | Small IBP systems generated by Tante (one-loop bubble, massless two-loop double box) | Cross-check against Kira ([arXiv:2008.06494](https://arxiv.org/abs/2008.06494)) or FIRE ([arXiv:2311.02370](https://arxiv.org/abs/2311.02370)) | Master-integral coefficients from Kira or FIRE | Sparse symbolic Gaussian elimination; solution families | Generated in-house | No |
| C3.6 | 52×52 linear system timing (FERMAT ~40 s vs "multiple minutes" in Mathematica) | [arXiv:2512.19858](https://arxiv.org/abs/2512.19858), sec. 5.7 | Timing only | Performance context | Not redistributable (system not published) | Primary |

### Stage 4: indices and tensors

| ID | Entry | Source | Expected result | Establishes | Licence / redistributable | Verified |
|---|---|---|---|---|---|---|
| C4.1 | Cadabra `meld` Riemann–Riemann example | [Cadabra manual: meld](https://cadabra.science/manual/meld.html) | One term with coefficient 3/2 (exact input to be transcribed) | Multi-term symmetry reduction | Manual GPL-3.0 project; restate | Primary (coefficient only) |
| C4.2 | Gauss–Bonnet combination R² − 4R_{ab}R^{ab} + R_{abcd}R^{abcd} as a total derivative in 4D | Invar [arXiv:0802.1274](https://arxiv.org/abs/0802.1274) | Dimension-dependent identity | Dimension-dependent identities | Fact | No |
| C4.3 | Counts of independent Riemann invariants per order | Invar [arXiv:0704.1756](https://arxiv.org/abs/0704.1756), [arXiv:0802.1274](https://arxiv.org/abs/0802.1274) | In paper tables (not extracted) | Completeness of multi-term canonicalisation | Fact | No |
| C4.4 | Highly symmetric tensor canonicalisation stress cases | Niehoff [arXiv:1702.08114](https://arxiv.org/abs/1702.08114) | Canonical forms; avoid O(n!) blow-up | Canonicaliser worst cases | Fact | Primary (claim) |
| C4.5 | Canonicalisation of about 100 indices in seconds | xPerm [arXiv:0803.0862](https://arxiv.org/abs/0803.0862) | Timing (2008 hardware) | Performance anchor | Fact | Primary |
| C4.6 | Equality up to free-index relabelling | Redberry [arXiv:1302.1219](https://arxiv.org/abs/1302.1219) | Signed mapping found or not | Tensor equality semantics | Fact | Primary |

### Stage 5: FORM front end

| ID | Entry | Source | Expected result | Establishes | Licence / redistributable | Verified |
|---|---|---|---|---|---|---|
| C5.1 | FORM `check/examples.frm` (128 manual examples) | [check/](https://github.com/form-dev/form/tree/master/check) | Asserted in each fold | Manual-documented semantics | GPL-3.0-or-later, vendorable | Primary (count) |
| C5.2 | FORM `check/fixes.frm` (250 regression tests) | same | Asserted in each fold | Edge cases and past bugs | GPL-3.0-or-later | Primary (count) |
| C5.3 | FORM `features.frm` (117), `coverage.frm` (13), `diagrams.frm` (8), `user.frm` (5) | same | Asserted | Newer features, including FORM 5 | GPL-3.0-or-later | Primary (count) |
| C5.4 | FORM `check/extra`: Mincer (4), Forcer (5), mzv-dm (10), z-fmft (3), color, multithreaded, series, checkpoint (1 each) | [check/extra](https://api.github.com/repos/form-dev/form/contents/check/extra) | Asserted | Physics-grade real programs | GPL-3.0-or-later; Forcer's own repo shows no licence | Primary (count) |
| C5.5 | Missing-semicolon misparse (`Symbol x1,x2` followed by `Index m1,m2;`) | [FORM manual](https://form-dev.github.io/form-docs/stable/manual/) sec. 2.13 | Tante must report an error ([REQ-024](requirements.md#req-024)); FORM silently declares symbols | Negative test of diagnostics | Fact | Primary |
| C5.6 | `once` match-order dependence | FORM manual, `identify` | Tante's result is specified and documented | Determinism of matching | Fact | Primary |

### Stage 6: physics modules

| ID | Entry | Source | Expected result | Establishes | Licence / redistributable | Verified |
|---|---|---|---|---|---|---|
| C6.1 | Dirac traces of two and four γ matrices | Textbook | tr(γ^μγ^ν)=4g^{μν}; tr(γ^μγ^νγ^ργ^σ)=4(g^{μν}g^{ρσ} − g^{μρ}g^{νσ} + g^{μσ}g^{νρ}) | Basic Dirac algebra | Fact | No |
| C6.2 | Tr(γ^μγ^νγ^ργ^σγ₅) under two Levi-Civita conventions | [FeynCalc `$LeviCivitaSign`](https://github.com/FeynCalc/feyncalc/blob/master/FeynCalc/Documentation/Markdown/$LeviCivitaSign.md) | −4i ε^{μνρσ} (FeynCalc default) vs the FORM convention | Convention recording ([REQ-034](requirements.md#req-034)) | Fact | Primary |
| C6.3 | FORM `trace4` with ≥10 γ matrices in n dimensions | [FORM manual gamma.tex](https://github.com/form-dev/form/blob/master/doc/manual/gamma.tex) | FORM documents wrong results; Tante must not reproduce this ([REQ-004](requirements.md#req-004)) | Known FORM defect as a negative test | Fact | Primary |
| C6.4 | NDR γ₅-odd traces ambiguous at O(D−4) | [arXiv:2001.04407](https://arxiv.org/abs/2001.04407) sec. 4.2 | Left unevaluated (or flagged), not silently fixed | γ₅ scheme contract | Fact | Primary |
| C6.5 | SU(N) invariants | [hep-ph/9802376](https://arxiv.org/abs/hep-ph/9802376) App. D | C_F=(N²−1)/(2N), C_A=N, T_F=½; d_F^{abcd}d_F^{abcd} = (N²−1)(N⁴−6N²+18)/(96N²) | Colour module | Fact | d44: Primary (eq. 194, survey's own arithmetic at a=½); others: No |
| C6.6 | Group-independent trace reductions, e.g. n=2: N_A I₂(R)(C_R − C_A/2); n=3: N_A I₂(R)(C_R − C_A)(C_R − C_A/2) | [hep-ph/9802376](https://arxiv.org/abs/hep-ph/9802376) App. C | As given; n=4 coefficient garbled in extraction, re-read Table 11 | Colour reduction algorithm | Fact; FORM `color.frm` GPL-3.0 | Primary (n=2,3) |
| C6.7 | Schwinger term | Schwinger, Phys. Rev. 73 (1948) 416, as checked in [El-GaEl.m](https://github.com/FeynCalc/feyncalc/blob/master/FeynCalc/Examples/QED/OneLoop/Mathematica/El-GaEl.m) | a_e = α/(2π) | End-to-end one-loop QED | Fact; FeynCalc script GPL-3 | Primary (FeynCalc check) |
| C6.8 | One-loop QCD renormalisation constants | Muta, eqs. 2.5.131–2.5.147, via [FeynCalc example](https://github.com/FeynCalc/feyncalc/blob/master/FeynCalc/Examples/QCD/OneLoop/Mathematica/Renormalization.m) | As in Muta | Renormalisation workflow | Textbook copyrighted: restate | Primary (citation in script) |
| C6.9 | Two-loop massless QED electron and photon self-energies; QCD gluon self-energy | [arXiv:2312.14089](https://arxiv.org/abs/2312.14089) sec. 7 (Grozin eqs. 5.51, 5.18, 6.10–6.11) | As in Grozin | Two-loop algebra | Fact | Primary (agreement stated) |
| C6.10 | σ(e⁺e⁻→μ⁺μ⁻), massless | Peskin & Schroeder §5.1 | 4πα²/(3s) | Tree-level amplitude and spin sums | Fact | No |
| C6.11 | One-loop β₀ | [Wikipedia](https://en.wikipedia.org/wiki/Beta_function_(physics)) | β₀ = 11 − 2n_f/3, with β(g) = −β₀g³/(16π²) | Renormalisation | Fact | Secondary |
| C6.12 | β₁, β₂ (a_s = α_s/4π) | [hep-ph/9701390](https://arxiv.org/abs/hep-ph/9701390) and earlier | β₁ = 102 − 38n_f/3; β₂ = 2857/2 − 5033n_f/18 + 325n_f²/54 | Multi-loop results | Fact | No |
| C6.13 | Four- and five-loop β function (consistency check only) | [hep-ph/9701390](https://arxiv.org/abs/hep-ph/9701390); BCK [arXiv:1606.08659](https://arxiv.org/abs/1606.08659); HRUVV [arXiv:1701.01404](https://arxiv.org/abs/1701.01404); LMMS [arXiv:1709.07718](https://arxiv.org/abs/1709.07718) | Load the published values and check group-invariant consistency | Long-term target; three independent five-loop results | Fact | Primary (papers exist); values not extracted |
| C6.14 | Three-loop splitting functions | MVV [hep-ph/0403192](https://arxiv.org/abs/hep-ph/0403192), [hep-ph/0404111](https://arxiv.org/abs/hep-ph/0404111) | In papers | FORM-era flagship results | Fact | No (not fetched) |
| C6.15 | Schwarzschild: Ricci-flat; Kretschmann scalar | [Wikipedia](https://en.wikipedia.org/wiki/Kretschmann_scalar) | R_ab = 0; K = 48G²M²/(c⁴r⁶) | GR module | Fact | Secondary |
| C6.16 | FLRW Kretschmann scalar | same | K = 12[a²ä² + (k + ȧ²)²]/a⁴ | GR with time dependence | Fact | Secondary |
| C6.17 | Kerr Kretschmann scalar (G=c=1) | Henry, [astro-ph/9912320](https://arxiv.org/abs/astro-ph/9912320) | 48M²(r⁶ − 15a²r⁴cos²θ + 15a⁴r²cos⁴θ − a⁶cos⁶θ)/(r² + a²cos²θ)⁶ | Heavier GR expression | Fact | No |
| C6.18 | Second-order perturbation of curvature | xPert [arXiv:0807.0824](https://arxiv.org/abs/0807.0824) | In paper | Perturbation theory | Fact | No |
| C6.19 | One-loop divergences of a minimal vector field | Pronin–Stepanyantz [hep-th/9605206](https://arxiv.org/abs/hep-th/9605206), as checked in Redberry [arXiv:1302.1219](https://arxiv.org/abs/1302.1219) | Published divergent action | Background-field one-loop algebra | Fact | Primary (Redberry's check) |
| C6.20 | Parke–Taylor MHV amplitude | [doi:10.1103/PhysRevLett.56.2459](https://doi.org/10.1103/PhysRevLett.56.2459) | ⟨ij⟩⁴/(⟨12⟩⟨23⟩…⟨n1⟩) | Spinor-helicity module | Fact | No |
| C6.21 | H→γγ regularisation subtlety | Weinzierl [arXiv:1402.4407](https://arxiv.org/abs/1402.4407); [FeynCalc example](https://github.com/FeynCalc/feyncalc/blob/master/FeynCalc/Examples/Misc/EWHiggsDecayControversy.m) | As resolved there | D-dimensional subtleties | Fact | Primary (exists) |
| C6.22 | MZV data mine | Blümlein–Broadhurst–Vermaseren [arXiv:0907.2557](https://arxiv.org/abs/0907.2557); FORM `check/extra/mzv-dm` | Bases to weight 26–28 | Large-table, large-scale workload | Data licence unchecked; FORM tests GPL-3.0 | Primary (paper) |

## 6. Open gaps

Some gaps concern Symbolica and licences:

- I did not establish whether Symbolica versions released before 15 September 2026 keep their earlier, less restrictive terms.
- The project has not taken legal advice on whether Tante is a "Competing Product".
- Still unverified: GiNaC's exact GPL version, xAct's licence version, SymPy's BSD-3 status, FLINT's licence, the licence of the standalone `xperm.c`, the Forcer repository's licence, and Redberry `core`'s GPL-2.0 versus GPL-3.0 conflict.

Some gaps concern FORM:

- I did not run FORM's test suite, so I have no pass rates or runtimes, and I did not retrieve its Coveralls coverage.
- I found no modern TFORM scaling curve beyond 8 cores.
- I found no statement on whether FORM and TFORM output are bit-identical.
- I found no single authoritative description of the term layout.
- I did not sample the 144 open issues by category.
- I did not confirm the minimum FLINT version after 5.0.2.

Several corpus entries still need their expected values taken from primary sources:

- β₁ to β₄
- the LO and three-loop splitting functions
- the e⁺e⁻→μ⁺μ⁻ squared amplitude (only the cross-section, from background knowledge, is recorded)
- Invar's invariant counts
- the colour-trace coefficient at n=4 and the d33 layout
- the term counts of C1.2 and C1.3
- polybench's generator definitions

[REQ-070](requirements.md#req-070)'s Unruh-temperature entry has no source in this survey. No public, licensed table of ε-expansions was found, and no versioned symbolic-determinant benchmark suite was found.

Coverage of the other systems is thin in places. SymPy internals and pattern matching, xTensor's rule mechanism, Cadabra's `meld` algorithm and its current use of xPerm, GiNaC's thread safety, and SymEngine's OpenMP scope were not sourced. The FeynCalc test count (~14,500) is a regex count, not a published figure. No independent user studies of learnability exist for any system surveyed; the complaints about FORM come from developer talks and papers.

## 7. References

**FORM.**
[form-dev/form](https://github.com/form-dev/form) ·
[releases](https://github.com/form-dev/form/releases) ·
[check/](https://github.com/form-dev/form/tree/master/check) ·
[check/README.md](https://github.com/form-dev/form/blob/master/check/README.md) ·
[test.yml](https://github.com/form-dev/form/blob/master/.github/workflows/test.yml) ·
[deploy.yml](https://github.com/form-dev/form/blob/master/.github/workflows/deploy.yml) ·
[manual](https://form-dev.github.io/form-docs/stable/manual/) ·
[gamma.tex](https://github.com/form-dev/form/blob/master/doc/manual/gamma.tex) ·
[tutorial](https://www.nikhef.nl/~form/maindir/documentation/tutorial/online/online.html)

Version papers:
- Vermaseren, "New features of FORM", [math-ph/0010025](https://arxiv.org/abs/math-ph/0010025) (2000)
- Vermaseren, "FORM facts", [arXiv:1006.4512](https://arxiv.org/abs/1006.4512) (2010)
- Kuipers, Ueda, Vermaseren, Vollinga, "FORM version 4.0", [arXiv:1203.6543](https://arxiv.org/abs/1203.6543), CPC 184 (2013) 1453
- "Code Optimization in FORM", [arXiv:1310.7007](https://arxiv.org/abs/1310.7007) (2013)
- Ruijl, Ueda, Vermaseren, "FORM version 4.2", [arXiv:1707.06453](https://arxiv.org/abs/1707.06453) (2017)
- Davies, Kaneko, Marinissen, Ueda, Vermaseren, "FORM Version 5.0", [arXiv:2601.19982](https://arxiv.org/abs/2601.19982) (2026)

Parallel versions:
- Tentyukov & Vermaseren, TFORM, [hep-ph/0702279](https://arxiv.org/abs/hep-ph/0702279), CPC 181 (2010) 1419
- ParFORM, [cs/0407066](https://arxiv.org/abs/cs/0407066)

Issues: [#623](https://github.com/form-dev/form/issues/623) · [#624](https://github.com/form-dev/form/issues/624) · [#625](https://github.com/form-dev/form/issues/625) · [#626](https://github.com/form-dev/form/issues/626)

Talks:
- Ruijl, "The future of FORM", 13 April 2023 ([slides](https://indico.ift.uam-csic.es/event/17/contributions/207/attachments/144/217/symbolica.pdf); used for its statements on FORM only)
- Chargeishvili, CAPP 2025 ([slides](https://indico.desy.de/event/46419/contributions/175710/attachments/92426/132204/FORM_day1.pdf))

**FORM-based physics.**
[hep-ph/9302208](https://arxiv.org/abs/hep-ph/9302208) ·
[hep-ph/9701390](https://arxiv.org/abs/hep-ph/9701390) ·
[hep-ph/0403192](https://arxiv.org/abs/hep-ph/0403192) ·
[hep-ph/0404111](https://arxiv.org/abs/hep-ph/0404111) ·
[arXiv:1606.08659](https://arxiv.org/abs/1606.08659) ·
[arXiv:1701.01404](https://arxiv.org/abs/1701.01404) ·
[arXiv:1709.07718](https://arxiv.org/abs/1709.07718) ·
[arXiv:1704.06650](https://arxiv.org/abs/1704.06650) ·
[arXiv:1707.08315](https://arxiv.org/abs/1707.08315) ·
[arXiv:0907.2557](https://arxiv.org/abs/0907.2557) ·
[hep-ph/9802376](https://arxiv.org/abs/hep-ph/9802376)

**GiNaC.**
[ginac.de](https://www.ginac.de/) ·
[tutorial](https://www.ginac.de/tutorial/) ·
[News](https://www.ginac.de/News.html) ·
[cs/0004015](https://arxiv.org/abs/cs/0004015) ·
[hep-ph/0410259](https://arxiv.org/abs/hep-ph/0410259) ·
[arXiv:2602.09956](https://arxiv.org/abs/2602.09956)

**SymEngine / SymPy.**
[symengine](https://github.com/symengine/symengine) ·
[dict.h](https://symengine.org/symengine/dict_8h.html) ·
[symengine.py](https://github.com/symengine/symengine.py) ·
[sympy](https://github.com/sympy/sympy) ·
[doi:10.7717/peerj-cs.103](https://doi.org/10.7717/peerj-cs.103)

**Symbolica (positioning and licence only).**
[GitHub](https://github.com/benruijl/symbolica) ·
[License.md](https://github.com/benruijl/symbolica/blob/main/License.md) ·
[License history](https://github.com/benruijl/symbolica/commits/main/License.md) ·
[releases](https://github.com/benruijl/symbolica/releases) ·
[licence terms](https://symbolica.io/license/) ·
[1.0 announcement](https://symbolica.io/posts/stable_release/index.html)

**Cadabra2.**
[GitHub](https://github.com/kpeeters/cadabra2) ·
[cadabra.science](https://cadabra.science/) ·
[canonicalise](https://cadabra.science/manual/canonicalise.html) ·
[meld](https://cadabra.science/manual/meld.html) ·
[cs/0608005](https://arxiv.org/abs/cs/0608005) ·
[hep-th/0701238](https://arxiv.org/abs/hep-th/0701238) ·
[arXiv:2210.00005](https://arxiv.org/abs/2210.00005) ·
[arXiv:2210.00007](https://arxiv.org/abs/2210.00007) ·
[JOSS](https://joss.theoj.org/papers/10.21105/joss.01118)

**Redberry.**
[arXiv:1302.1219](https://arxiv.org/abs/1302.1219) ·
[redberry-cas](https://github.com/redberry-cas) ·
[Maven](https://mvnrepository.com/artifact/cc.redberry/physics/versions) ·
[hep-th/9605206](https://arxiv.org/abs/hep-th/9605206)

**xAct and canonicalisation.**
[xact.es](http://www.xact.es/) ·
[arXiv:0803.0862](https://arxiv.org/abs/0803.0862) ·
[math-ph/0107031](https://arxiv.org/abs/math-ph/0107031) ·
[math-ph/0107032](https://arxiv.org/abs/math-ph/0107032) ·
[arXiv:1702.08114](https://arxiv.org/abs/1702.08114) ·
[arXiv:0704.1756](https://arxiv.org/abs/0704.1756) ·
[arXiv:0802.1274](https://arxiv.org/abs/0802.1274) ·
[arXiv:0807.0824](https://arxiv.org/abs/0807.0824)

**FeynCalc and related packages.**
[GitHub](https://github.com/FeynCalc/feyncalc) ·
[feyncalc.github.io](https://feyncalc.github.io/) ·
[arXiv:1601.01167](https://arxiv.org/abs/1601.01167) ·
[arXiv:2001.04407](https://arxiv.org/abs/2001.04407) ·
[arXiv:2312.14089](https://arxiv.org/abs/2312.14089) ·
[arXiv:2512.19858](https://arxiv.org/abs/2512.19858) ·
[arXiv:1212.3522](https://arxiv.org/abs/1212.3522) ·
[feynarts.de](https://feynarts.de/) ·
[arXiv:1503.01469](https://arxiv.org/abs/1503.01469) ·
[arXiv:1412.6102](https://arxiv.org/abs/1412.6102) ·
[arXiv:1610.09331](https://arxiv.org/abs/1610.09331) ·
[arXiv:2103.08507](https://arxiv.org/abs/2103.08507) ·
[arXiv:1209.6213](https://arxiv.org/abs/1209.6213) ·
[arXiv:1402.4407](https://arxiv.org/abs/1402.4407)

**Benchmarks and corpus sources.**
[Monagan–Pearce, Maple 14](https://cecm-www.dcr.sfu.ca/personal/monaganm/papers/Maple14.pdf) ·
[doi:10.1145/844076.844080](https://doi.org/10.1145/844076.844080) ·
[arXiv:1705.06134](https://arxiv.org/abs/1705.06134) ·
[symbench](https://wiki.sagemath.org/symbench) ·
[polybench](https://github.com/tueda/polybench/) ·
[Wester](https://math.unm.edu/~wester/cas_review.html) ·
[arXiv:2304.13418](https://arxiv.org/abs/2304.13418) ·
[arXiv:2311.02370](https://arxiv.org/abs/2311.02370) ·
[arXiv:2008.06494](https://arxiv.org/abs/2008.06494) ·
[arXiv:2510.07150](https://arxiv.org/abs/2510.07150) ·
[hep-ph/0507094](https://arxiv.org/abs/hep-ph/0507094) ·
[arXiv:0708.2443](https://arxiv.org/abs/0708.2443) ·
[DLMF 5.7.3](https://dlmf.nist.gov/5.7.E3) ·
[Kretschmann scalar](https://en.wikipedia.org/wiki/Kretschmann_scalar) ·
[astro-ph/9912320](https://arxiv.org/abs/astro-ph/9912320) ·
[doi:10.1103/PhysRevLett.56.2459](https://doi.org/10.1103/PhysRevLett.56.2459) ·
[Beta function](https://en.wikipedia.org/wiki/Beta_function_(physics))
