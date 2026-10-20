# ADR-0003: Polynomial arithmetic uses FLINT, integers GMP and numerical evaluation FLINT's ball arithmetic (arb), each behind an interface of Tante's own, with a simple implementation of Tante's own for cross-checks (resolves OPEN-O (a))

- **Status:** Accepted (Thomas, 16 October 2026); §1's version-update rule clarified in place, 20 October 2026
- **Date:** 2026-10-16

## Context

[REQ-010](../requirements.md#req-010) requires, in the first release, polynomial greatest common divisors, factorisation over the rationals in any number of symbols, and normalisation of rational functions, all with exact rational coefficients of arbitrary size. [REQ-056](../requirements.md#req-056) (B) asks that polynomial arithmetic be within a stated factor of FORM's, and FORM 5 does it with FLINT. [REQ-101](../requirements.md#req-101) asks for numerical evaluation to a chosen precision with every printed digit correct.

[OPEN-O](../requirements.md#open-o) (a) asked whether an arithmetic library counts as "another computer-algebra system's core", which [REQ-004](../requirements.md#req-004) forbids building on. Thomas resolved it on 15 October 2026: arithmetic libraries may be used, each behind an interface of the engine's own ([REQ-096](../requirements.md#req-096)), with an independent polynomial implementation for cross-checks ([REQ-097](../requirements.md#req-097)); FLINT's licence and version were to be confirmed in this record. [REQ-074](../requirements.md#req-074) (d) requires that the result be identical whichever polynomial implementation is selected.

Evidence, checked on 16 October 2026:

- **FLINT 3.6.0**, released 29 June 2026 ([releases](https://github.com/flintlib/flint/releases)), is the newest release and the version in vcpkg at the baseline pinned by the build spike (vcpkg commit `c52f7bd7`). Its licence is the **GNU LGPL, version 3 or later** (`LGPL-3.0-or-later`): stated in its [README at tag v3.6.0](https://github.com/flintlib/flint/blob/v3.6.0/README.md#license) and in the header of each source file (for example [`src/fmpz_mpoly/mul.c`](https://github.com/flintlib/flint/blob/v3.6.0/src/fmpz_mpoly/mul.c)). vcpkg's port metadata gives `GPL-3.0-only`; that field is informational and wrong, and the upstream statement governs. FLINT 3 also contains the former Arb library (ball arithmetic with rigorous error bounds) and Calcium.
- **GMP 6.3.0** (the vcpkg version) is dual-licensed: the LGPL version 3 or the GPL version 2, at the recipient's choice, and later versions of either ([GMP manual, Copying](https://gmplib.org/manual/Copying)), that is `LGPL-3.0-or-later OR GPL-2.0-or-later`.
- **MPFR 4.2.2**, released 20 March 2025 and the vcpkg version, is distributed under the **LGPL version 3 or later** ([mpfr.org](https://www.mpfr.org/)).
- All three licences are compatible with Tante's GPL-3.0-or-later ([REQ-080](../requirements.md#req-080)): the LGPL-3.0 permits combination with a GPL-3.0 work, and the combined work is distributed under the GPL-3.0 with complete source, which meets the LGPL's conditions whether the libraries are linked statically or dynamically.
- FORM 5 converts its terms to FLINT polynomials and back, and reports speed-ups of 16× to 53× on polynomial-heavy work; SymEngine also links FLINT ([prior-art.md §4.1](../prior-art.md#41-a-new-and-independent-engine-req-004), [arXiv:2601.19982](https://arxiv.org/abs/2601.19982)).
- The [native arithmetic build spike](../spikes/2026-10-16-native-arithmetic-build.md) of 16 October 2026 built all three libraries through vcpkg with MSVC, GCC and Apple Clang, and got byte-identical results on Windows, Linux and macOS for polynomial multiplication, GCD, factorisation over the rationals (checked by multiplying back) and an MPFR evaluation of exp(1) to 30 digits. It also showed that FLINT's `ulong` and `slong` are 64 bits on all three while C's `long` is 32 bits on Windows, and that FLINT returns the factors of a factorisation in an order of its own rather than a sorted one.

## Decision

**Tante's polynomial operations go through an interface of Tante's own with two implementations: one backed by FLINT, the default, and a simple one written in Tante, selectable by the user. Big integers use GMP, and numerical evaluation uses FLINT's ball arithmetic (`arb`), each through its own Tante interface. No code outside an adapter names a FLINT, GMP or MPFR type or function, and the result is normalised by Tante, not by the library, so it is identical whichever implementation computed it.** The parts:

### 1. Libraries, versions and licences

| Library | Version | Licence | Used for |
|---|---|---|---|
| FLINT | 3.6.0 (minimum 3.6.0) | LGPL-3.0-or-later | multivariate polynomial multiplication, exact division, GCD, factorisation over the rationals |
| GMP | 6.3.0 | LGPL-3.0-or-later OR GPL-2.0-or-later | arbitrary-size integers and rationals (also FLINT's own base) |
| FLINT's `arb` | part of FLINT 3.6.0 | LGPL-3.0-or-later | ball arithmetic: real numbers with a guaranteed error bound, for numerical evaluation ([REQ-101](../requirements.md#req-101)) |
| MPFR | 4.2.2 | LGPL-3.0-or-later | a dependency of FLINT, which builds on it; not called by Tante directly |

- Versions are fixed by the vcpkg `builtin-baseline` of [ADR-0001](0001-cpp20-cmake-vcpkg-guardrails.md) §2. Moving to a new version is a deliberate commit (`build(deps): …`) that must pass the whole polynomial corpus with both implementations; a change of output across a version update fails the tests and is investigated. IF independent evidence (Tante's own implementation, multiplying back, or the corpus entry's source) shows that the new output is correct, THEN the expected result is corrected in the same commit, with that evidence and a reference to the upstream fix; it is never updated merely because the new version produced it. *(Clarified 20 October 2026 with Thomas: the earlier wording, "a failed test, not a new golden result", could be read as blocking a version that fixes a wrong result; the intent was always to forbid accepting changed output without evidence.)*
- Binary distributions (release archives, and later the Python wheels of [REQ-062](../requirements.md#req-062)) include the licence texts of all three libraries and state where their source can be obtained.

### 2. The interfaces

In .NET terms each interface is an abstract base class (or an `interface`) that the engine is written against, with the library-specific class supplied at start-up, the way a service is registered in dependency injection. In C++ this is a class with pure virtual functions; each adapter is a separate library target, so that the build itself enforces [REQ-096](../requirements.md#req-096)'s rule that nothing else includes FLINT's headers.

- **`PolynomialBackend`**: multiplication, exact division (failing if not exact), division with remainder in a chosen variable, greatest common divisor, and factorisation over the rationals, on polynomials with rational coefficients in the symbols Tante passes in. Factorisation is an optional capability: an implementation may say it does not provide it.
- **Conversion at the boundary.** Polynomials are passed in and out in a neutral form defined by Tante (a list of monomials, each an exponent vector and a rational coefficient, over an ordered list of Tante symbols). The engine's own term representation (the record on term representation) is converted to and from this form; FLINT's `fmpz_mpoly` and `fmpq_mpoly` objects exist only inside the FLINT adapter, wrapped in classes whose destructor frees them (RAII, [ADR-0001](0001-cpp20-cmake-vcpkg-guardrails.md) §3), as the spike's `Poly` class does. This is the same conversion FORM 5 makes.
- **`Integer` and `Rational`**: Tante's own value types for coefficients, backed by GMP. How they are laid out in memory (for example whether small values avoid GMP altogether) is left to the record on term representation (Thomas, 16 October 2026). Only the integer adapter includes `gmp.h`. FLINT's own `fmpz` is not used as Tante's integer type, so that replacing FLINT does not change the representation of coefficients.
- **`RealBall`**: a real number known to lie within a guaranteed error bound, for [REQ-101](../requirements.md#req-101), backed by FLINT's `arb_t` (a midpoint, a radius and a working precision in bits). Only the evaluation adapter includes FLINT's `arb` headers.

### 3. Tante's own polynomial implementation ([REQ-097](../requirements.md#req-097))

- A deliberately simple implementation of `PolynomialBackend` over Tante's `Integer` and `Rational`: sparse polynomials as sorted lists of terms; schoolbook multiplication; exact and remainder division by the classical algorithm; GCD by the primitive polynomial remainder sequence, recursively in one variable at a time with coefficients in the others. It aims at being obviously correct, not fast, and shares no code with the FLINT adapter except the boundary types.
- It does **not** provide factorisation. Factorisation from FLINT is checked by multiplying the factors back and comparing ([REQ-010](../requirements.md#req-010), [OPEN-Q](../requirements.md#open-q)), which shows the factors are correct but not that they are irreducible; the polynomial corpus entries with known factorisations ([prior-art.md §5](../prior-art.md#5-validation-corpus-catalogue), entry C1.4) check irreducibility on those cases.
- The user selects the implementation per run (a command-line option and a script setting; their exact spelling belongs to the interface design), and the provenance record ([REQ-075](../requirements.md#req-075)) names the implementation and library versions used.
- The automated tests run every polynomial corpus entry with both implementations and require byte-identical output ([REQ-097](../requirements.md#req-097), [REQ-074](../requirements.md#req-074) (d)). On entries too large for the simple implementation, the test runs a reduced size of the same problem and says so.

### 4. Identical results whichever implementation ([REQ-074](../requirements.md#req-074) (d))

Libraries leave some choices open: the sign and scaling of a GCD, the order in which factors are returned, the unit split between a factorisation's constant and its factors, the variable order of their internal representation. Tante does not inherit these choices:

- **Variables** are passed to the back end in Tante's canonical order of symbols ([REQ-012](../requirements.md#req-012)), never in order of first appearance or by pointer value.
- **Every result is normalised by Tante after it returns**: a GCD is made primitive with a positive leading coefficient in Tante's canonical order; a factorisation is written as a rational constant times primitive factors, each with a positive leading coefficient, the factors sorted in Tante's canonical order with their multiplicities; a rational function has a primitive denominator with a positive leading coefficient. The spike showed why: FLINT returns factors in an order of its own, which Tante sorts before printing.
- Both implementations pass through the same normalisation code, so a disagreement between them is a bug in one of them, never a difference of convention.

### 5. Threads, memory and global state

- **FLINT's own threads are not used.** Tante calls FLINT with one thread (`flint_set_num_threads(1)`) and parallelises over terms itself ([REQ-052](../requirements.md#req-052)), so that thread scheduling is under one scheduler and determinism ([REQ-074](../requirements.md#req-074) (a)) is argued in one place. Using FLINT's threads for a single very large GCD is a later optimisation and would be its own decision.
- **Out of memory.** GMP and FLINT abort the process when an allocation fails. C++ exceptions must not be thrown through their C code. How Tante reports exhausted memory ([REQ-024](../requirements.md#req-024)) is settled by the record on errors; until then an allocation failure inside a library ends the run with the library's message.
- **Per-thread caches**: FLINT keeps thread-local caches; each worker thread frees them when it ends (`flint_cleanup`), so that sanitizer leak checks stay clean.

### 6. Numerical evaluation ([REQ-101](../requirements.md#req-101))

- **Evaluation uses ball arithmetic** (Thomas, 16 October 2026). Each number is held as a midpoint and a radius, meaning the true value lies in [midpoint − radius, midpoint + radius], and every operation (including exp, log, sin, cos and powers, [REQ-099](../requirements.md#req-099)) returns a ball proven to contain the true result, rounding errors and the uncertainty of its inputs included.
- Evaluation substitutes exact values, converts them to balls at a working precision somewhat above the requested number of digits, and evaluates. **IF the whole result ball rounds to the same requested digits, THEN those digits are proven correct and are printed; otherwise the evaluator doubles the working precision and repeats.** It stops with a message after a precision limit the user can raise, rather than print a digit it cannot prove ([REQ-018](../requirements.md#req-018), [REQ-024](../requirements.md#req-024)). The precision used is reported with the result.
- Why not MPFR with repeated evaluation: MPFR rounds each single operation correctly but does not track how errors accumulate across an expression. Agreement of two evaluations at different precisions can be a coincidence: exp(10⁻⁵⁰) − 1 gives 0 at both 30 and 38 digits, while its true value is about 10⁻⁵⁰. Ball arithmetic gives 0 ± 10⁻³⁰ there, which shows that not even the first digit is known yet.
- The guarantee is as good as FLINT's `arb` implementation, which is widely used and tested; it is checked on corpus entries with known digits ([REQ-070](../requirements.md#req-070)).

## Alternatives considered

- **Only Tante's own polynomial arithmetic:** the most independent, and needed anyway for cross-checks. Rejected as the default: state-of-the-art multivariate GCD and factorisation are each years of specialist work, and [REQ-056](../requirements.md#req-056) (B) measures Tante against FORM 5, which uses FLINT.
- **Singular's factory library:** mature multivariate GCD and factorisation, but GPL-2-or-later C++ embedded in a whole computer-algebra system, much heavier to build natively on Windows, and closer to the "another system's core" that [REQ-004](../requirements.md#req-004) excludes. Not chosen; the strongest alternative for factorisation if FLINT ever fails.
- **NTL:** LGPL, fast for univariate polynomials, weak on sparse multivariate ones, which are what physics produces. Not chosen.
- **PARI/GP:** GPL-2-or-later, strong in number theory, not built for large sparse multivariate polynomials. Not chosen.
- **FLINT's `fmpz` as Tante's integer type:** saves a conversion, since FLINT uses it internally, but would tie coefficient storage to FLINT and break [REQ-096](../requirements.md#req-096) for integers. Not chosen.
- **Boost.Multiprecision instead of direct GMP and MPFR:** a C++ wrapper over the same libraries; adds a dependency without adding a capability Tante's own thin adapters lack. Not chosen.
- **MPFR with repeated evaluation at rising precision (Ziv's strategy):** the usual practice and slightly faster, but it checks the printed digits rather than proving them, and can be fooled by cancellation (§6). Rejected by Thomas, 16 October 2026, in favour of `arb`, which FLINT already provides at no extra dependency.
- **Symbolica:** excluded by [ADR-0002](0002-no-input-from-symbolica.md).

## Consequences

- Easier: FORM-class polynomial performance and factorisation in the first release without writing them; three mature, maintained libraries with a single build route ([ADR-0001](0001-cpp20-cmake-vcpkg-guardrails.md)).
- Easier: a library can be replaced, or Tante's own implementation made faster, without touching the engine, and the corpus proves the replacement.
- Harder: conversion at the boundary costs time on every polynomial operation; FORM 5 accepts the same cost. Measured when the first benchmarks of [REQ-056](../requirements.md#req-056) run.
- Harder: Tante's own implementation is a second codebase to maintain, and its GCD will be slow on large inputs, so cross-checks on large corpus entries use reduced sizes.
- Harder: normalisation after every library call is Tante's responsibility and must be tested against both implementations.
- Follow-up: the record on term representation defines how coefficients and monomials are held, and so the exact cost of conversion.
- Follow-up: the record on errors settles exhausted memory inside a library.
- Follow-up: the record on modules and the C interface decides whether compiled modules see `Integer` and `Rational` through the C interface ([REQ-032](../requirements.md#req-032)).
- Untested: the speed of Tante's own GCD; the cost of conversion at the boundary. (Factorisation into irreducibles over the rationals is unique up to signs and order, so after Tante's normalisation of §4 any correct library gives the same printed result.)
