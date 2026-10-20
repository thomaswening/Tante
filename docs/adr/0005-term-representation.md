# ADR-0005: A term is a flat, pointer-free block of 32-bit words with a typed coefficient, an expression is a stream of terms behind a storage interface, and only the core knows the layout

- **Status:** Proposed
- **Date:** 2026-10-21

## Context

Everything the engine does (expanding, rewriting, sorting, saving) acts on terms, so their representation is the decision hardest to change later. The requirements behind it:

- [REQ-054](../requirements.md#req-054): expressions larger than memory must be addable without restructuring, and no first-release interface may assume an expression is in memory as a whole. FORM manages this with self-contained terms that contain no pointers, statements that act on one term at a time, and an external merge sort (arXiv:1203.6543).
- [REQ-098](../requirements.md#req-098): no limit fixed in advance on the size of a term, a coefficient or an expression.
- [REQ-017](../requirements.md#req-017): indices, vectors and tensors arrive in the second release without restructuring the representation.
- [REQ-022](../requirements.md#req-022): both front ends produce the same kind of object.
- [REQ-052](../requirements.md#req-052) and [REQ-074](../requirements.md#req-074): several threads, with output that is identical whatever their number, so the order of terms must not depend on memory addresses or hash values.
- [REQ-010](../requirements.md#req-010): the first release normalises rational functions, so a coefficient can be a rational function of some symbols, not only a number.
- [ADR-0003](0003-flint-gmp-arb-behind-own-interfaces.md) §2 passes polynomials to the back ends in a neutral form (monomials as exponent vectors over an ordered list of symbols, with rational coefficients). [ADR-0004](0004-library-first-target-structure.md) made modules see terms only through the C module interface, and left the cost of a call per term to be measured here.

Vocabulary, for a .NET reader:

- A **word** is the fixed-size unsigned integer the encoding is counted in, here 32 bits (C#'s `uint`). Every field (a kind, a length, a symbol number, a power, a 32-bit chunk of a large number) is one word.
- A **view** is a read-only cursor over words someone else owns, like `ReadOnlySpan<byte>` read with `Utf8JsonReader`. A **builder** writes a new term, like `Utf8JsonWriter`. Neither owns or copies the term.
- **Type–length–value**: each block begins with its kind and its length, so a reader can skip a block it does not understand, as a `BinaryReader` skips a length-prefixed record.

The measurements are in the [term-representation spike](../spikes/2026-10-21-term-representation.md) (three CI runs on Windows, Linux and macOS, results byte-identical on all three):
- The flat 32-bit encoding was 1.6 to 4.4 times smaller than a tree of reference-counted nodes, and sorted 1.1 to 3.5 times faster.
- 64-bit words doubled the size and sorted 2 to 32 % slower.
- Comparing symbols through a table of ranks by name cost about 5 %.
- A module call through the C interface cost 1.2 to 1.8 times a direct call when it rewrote a term, failing the rule of 1.25 set before the run.

Thomas decided points 1, 2 and 6 and the spike on 20 and 21 October 2026, after a brief in chat, and points 3 and 7 on 21 October 2026, after the spike. Points 4, 5 and 8, and the signed powers and long forms of point 2, were proposed by Claude and listed to Thomas before this record was written (21 October 2026).

## Decision

**A term is one contiguous block of 32-bit words with no pointers, made of type–length–value blocks and ending in a coefficient that is either a rational number or a rational function. An expression is a stream of such terms behind a storage interface. Only the core's encoding classes know the layout; all other code, modules included, reads terms through views and writes them through builders.**

```
¾·x²·y·f(a + b)
[37]                                   length of the term in words
  [SYM   6 | x 2  y 1]                 symbols with their powers, in canonical order
  [FUN  25 | f | ARG 22 | [10 a·1] [10 b·1] ]  a function; each argument is an expression of whole terms
  [COEF  5 | 1 | 3 | 4]                coefficient: numerator limb count, numerator 3, denominator 4
```

1. **Flat and pointer-free (Thomas: option a).** A term is `[length] [SYM]? [factor blocks]* [coefficient]`; each block is `[kind, length, payload]`. Function arguments are expressions encoded inline as complete terms, at any depth. A term can be copied with `memcpy`, written to disk unchanged and compared by scanning words. A reader skips any block whose kind it does not handle.
2. **32-bit words (Thomas, confirmed by the spike).** In native byte order in memory (all supported platforms are little-endian); the saved format defines its own ([REQ-100](../requirements.md#req-100), the record on the binary save format). **Powers are signed** (1/x, Laurent series of [REQ-093](../requirements.md#req-093)). **No field caps a size** ([REQ-098](../requirements.md#req-098)): a length, power or count that does not fit in one word uses a long form (a marker word followed by a 64-bit value), and numbers have as many 32-bit limbs as they need.
3. **A typed coefficient, always the last block (Thomas, 21 October 2026).** Either `COEF`, a normalised rational number (numerator and denominator limbs, denominator positive, no common factor), or `RATFUN`, a numerator and a denominator polynomial in [ADR-0003](0003-flint-gmp-arb-behind-own-interfaces.md)'s neutral form over the symbols that belong in coefficients. The builder normalises a `RATFUN` through the polynomial back end when it writes the term, so the result is the same whichever back end is selected ([REQ-074](../requirements.md#req-074) (d)). Comparison of terms stops before the coefficient; merging adds the coefficients, whatever their kind. No function name is reserved for coefficients, unlike FORM's `PolyRatFun`.
4. **Names live in tables, not in terms.** Symbols and functions appear as numbers into the session's tables, which hold their names and declared properties (symmetries, commutation, dimensions). The numbers are given in creation order and are valid only in that session: they never decide the canonical order (the record on canonical order) and are never written to a file without the table of names (the record on the binary save format).
5. **An expression is a stream of terms.** The storage interface has three phases: append terms, sort them (merging equal terms), then read them in order. It offers no random access and no count of terms held in memory. Terms are immutable once written; a statement reads a term through a view and writes its results through a builder. `tante_store_memory` comes first; a store on disk is added for [REQ-054](../requirements.md#req-054) behind the same interface ([ADR-0004](0004-library-first-target-structure.md), appendix).
6. **Only the core knows the layout (Thomas: opaque handles).** One pair of classes in `tante_core`, the term view and the term builder, reads and writes the words. Modules see terms as opaque handles with accessor functions in the C module interface; the layout is part of no public contract, so it can change within a major version.
7. **Built-in modules call the core directly (Thomas, 21 October 2026).** Because the C interface failed the 1.25 rule, [ADR-0004](0004-library-first-target-structure.md)'s remedy applies from the start: `tante/module.hpp` has a second implementation that calls the core directly, and the built-in modules compile against it. CI also builds and tests them against the C implementation, so the C interface stays exercised. Third-party modules pay the measured cost.
8. **Room for indices and tensors ([REQ-017](../requirements.md#req-017)).** Block kinds are reserved now for indices, vectors, and tensors (functions whose symmetry and commutation properties live in the tables of point 4). Their contents are defined when the second release adds them; a first-release reader skips them by their length.

## Alternatives considered

- **A tree of shared, reference-counted nodes** (as in GiNaC and SymEngine): natural to program, and it shares common subterms. Not chosen: pointers cannot be written to disk ([REQ-054](../requirements.md#req-054)); reference counts are contended between threads; it measured 1.6 to 4.4 times larger and 1.1 to 3.5 times slower to sort. Hash-consing and pool allocation would narrow the gap without removing the pointers.
- **Trees in memory, encoded only for disk:** two representations, with conversion at every boundary, which in FORM's design is the most frequent path.
- **64-bit words:** fields rarely need more than 32 bits, and the long forms of point 2 cover those that do; 64-bit words doubled the size and slowed sorting by 2 to 32 %.
- **The layout published in the C contract**, so that modules read words directly: the fastest possible access, but it would freeze the encoding for a whole major version ([REQ-031](../requirements.md#req-031)). Point 7 gives built-in modules the speed without the freeze.
- **A rational-function coefficient as a reserved function** (FORM's `PolyRatFun`): no new block kind, but every piece of code that walks factors (sorting, pattern matching, printing, differentiation, modules) must treat one function as not a factor, and the term carries two coefficients.
- **Bulk accessors in the C interface** (one call per term instead of one per symbol): measured, and did not reliably close the gap on macOS and Windows. The wrapper can still adopt them later without changing module code.

## Consequences

- Easier: saving, sorting on disk and passing terms between threads are copies of words; reading a term needs no allocation; a reader written today skips block kinds added later.
- Easier: modules and pattern matching see only real factors, and the coefficient is one typed object.
- Harder: changing a term means writing a new one, and engine code works with views and builders rather than plain objects. The long forms of point 2 add a branch to every field read, kept to the encoding classes.
- Harder: two coefficient kinds, and adding a `COEF` to a `RATFUN` promotes the number to a constant rational function.
- Harder: two wrapper implementations to keep in step, mitigated by building the built-in modules against both in CI.
- Untested, from the spike: the real canonical order; negative and fractional coefficients through GMP; the size and comparison cost of index and tensor blocks; threads and disk; module functions doing real work per term.
- Follow-up: the record on canonical order fixes the order of blocks and fields, and whether symbols are compared through the rank table on every comparison or renumbered in name order at each sort. The record on the execution model fixes which symbols belong in a `RATFUN`, when normalisation runs and the threading contract of views and builders. The record on pattern matching matches over views. The record on the binary save format fixes byte order and the table of names. The record on errors fixes status codes at the C boundary.
- Follow-up: the C module interface gains the term accessors and builder functions measured in the spike; they are added with the first module, by need ([ADR-0004](0004-library-first-target-structure.md)).
