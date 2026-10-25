# Tante: Glossary

This glossary explains the terms a reader of Tante's documents may not know. Each entry is tagged by where the term comes from:

- *domain*: mathematics or physics, found in textbooks;
- *engineering*: software engineering, found in programming references;
- *project*: coined by Tante, or used by Tante in a sense of its own.

A document links a term on its first use, e.g. `[word](glossary.md#word)`. Anchors never change: a renamed term keeps its old anchor. Where [requirements.md](requirements.md) defines a term bindingly, the entry here is a one-line gloss and the requirements' wording governs.

Fixed labels after an entry relate it to others: **Kind of** names the broader term, **See also** related terms, **Not to be confused with** a look-alike with another meaning, **Defined in** the document whose wording governs, **After FORM** what Tante took from FORM and what it changed.

<a id="ball-arithmetic"></a>**ball arithmetic** *(domain)*: arithmetic on intervals written as a midpoint and a radius, where every result is guaranteed to contain the exact value. Example: $\pi$ as $[3.14159265 \pm 4 \times 10^{-9}]$. Tante evaluates numerically through FLINT's ball arithmetic (formerly the Arb library).

- Defined in: [ADR-0003](adr/0003-flint-gmp-arb-behind-own-interfaces.md)

<a id="block"></a>**block** (`SYM`, `FUN`, `COEF`, …) *(project)*: one part of an encoded [term](#term), a sequence of [words](#word) that begins with its kind and its length. `SYM` holds symbols with their powers, `FUN` a function with its arguments, `COEF` or [`RATFUN`](#ratfun) the coefficient. Example: in $\tfrac34 x^2 y$, the block `SYM · 6 · x · 2 · y · 1` holds $x^2 y$.

- Kind of: [type–length–value](#type-length-value)
- Defined in: [ADR-0005](adr/0005-term-representation.md#decision)
- After FORM: FORM also encodes a term as a length, then subterms that each begin with a type and a length, then the coefficient ([prior-art.md §2.1](prior-art.md#21-form-streaming-terms-and-a-disk-sort-now-with-flint)). Tante keeps the idea. Its coefficient is a typed block of its own.

<a id="builder"></a>**builder** *(project)*: the class in the core that writes a new term word by word. It is the only way to create a term. In .NET terms it plays the part of `Utf8JsonWriter`.

- See also: [view](#view)
- Defined in: [ADR-0005](adr/0005-term-representation.md)

<a id="canonical-form"></a>**canonical form** *(domain)*: a way of writing objects such that two objects are equal exactly when they are written identically. Example: $(x+1)^2$ and $x^2 + 2x + 1$ are written as $x^2 + 2x + 1$.

- Defined in: [REQ-012](requirements.md#req-012)

<a id="checkpoint"></a>**checkpoint** *(project)*: a point in a calculation the user marks, at which the engine records the expression so it can be shown, explained or compared.

- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions)

<a id="coefficient"></a>**coefficient** *(domain)*: the factor of a term that is not a symbol or function. Example: in $\tfrac34 x^2 y$, the coefficient is $\tfrac34$. In Tante, a coefficient is either a rational number or a [rational function](#rational-function). It is always the last [block](#block) of a term.

- See also: [RATFUN](#ratfun), [term](#term)

<a id="corpus"></a>**corpus** (the corpus) *(project)*: the set of calculations with independently known results that Tante's correctness is checked against. Example: the Schwarzschild metric is [Ricci-flat](#ricci-flat).

- Not to be confused with: [the FORM corpus](#form-corpus)
- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions); [REQ-070](requirements.md#req-070)

<a id="expression"></a>**expression** *(project)*: a mathematical object the engine holds and transforms: a sum of [terms](#term). Example: $x^2 + 2x + 1$ has three terms. In memory and on disk, an expression is a stream of terms behind a storage interface.

- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions); [ADR-0005](adr/0005-term-representation.md#decision), point 5

<a id="field-and-number"></a>**field and number** *(project)*: two different things in the term encoding. A *field* describes structure (a length, a count, a symbol number, a power) and normally takes one [word](#word). A *number* is a value such as a coefficient, stored as as many [limbs](#limb) as it needs. Example: $3/4$ takes one limb each for numerator and denominator.

- See also: [long form](#long-form)
- Defined in: [ADR-0005](adr/0005-term-representation.md)

<a id="form-corpus"></a>**FORM corpus** (the FORM corpus) *(project)*: a set of publicly available FORM scripts with FORM's output for each, used to measure how well Tante runs FORM scripts.

- Not to be confused with: [the corpus](#corpus)
- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions); [REQ-021](requirements.md#req-021)

<a id="hash-consing"></a>**hash-consing** *(engineering)*: keeping a hash table of every node built, so that each distinct subexpression exists once and is shared. *Cons* is Lisp's word for building a node. It is like `string.Intern` in .NET. Tante does not use it.

- Defined in: [term-representation spike](spikes/2026-10-21-term-representation.md#terms-used)

<a id="include-check"></a>**include check** *(engineering)*: a script (`tools/check_architecture.py`) that fails CI on any `#include` a directory may not use. It is needed because vcpkg installs all libraries' headers in one folder.

- Defined in: [ADR-0004](adr/0004-library-first-target-structure.md)

<a id="inlining"></a>**inlining** *(engineering)*: the compiler copies a function's body into its caller, so no call happens at run time. It is impossible across a shared-library boundary, since neither side's compiler sees the other side's code.

- Defined in: [term-representation spike](spikes/2026-10-21-term-representation.md#terms-used)

<a id="laurent-series"></a>**Laurent series** *(domain)*: a power series that may also contain negative powers. Example: $\frac{1}{\sin x} = x^{-1} + \frac{x}{6} + \frac{7x^3}{360} + \dots$ It is the reason powers in a term are signed.

- See also: [REQ-093](requirements.md#req-093)

<a id="limb"></a>**limb** *(engineering)*: one word-sized digit of a large number. Example: a 96-digit integer takes ten 32-bit limbs.

- See also: [word](#word)
- Defined in: [term-representation spike](spikes/2026-10-21-term-representation.md#terms-used)

<a id="long-form"></a>**long form** *(project)*: how a [field](#field-and-number) too large for one word is written: a marker word, then two words holding a 64-bit value. Example: a power of $2^{40}$. No field ever needs more, since no memory or disk holds $2^{63}$ words.

- Defined in: [ADR-0005](adr/0005-term-representation.md#decision), point 2

<a id="module"></a>**module** *(project)*: a unit of capability (new kinds of object, rules, functions, conventions) added to the engine through its public extension interface. The engine's own domains beyond the core are modules too.

- See also: [module interface](#module-interface)
- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions)

<a id="module-interface"></a>**module interface** (C module interface) *(project)*: the one C interface through which every [module](#module), built-in or third-party, connects to the engine. Modules see terms through it only as [opaque handles](#opaque-handle). Built-in modules compile against a second implementation that calls the core directly.

- Defined in: [ADR-0004](adr/0004-library-first-target-structure.md); [ADR-0005](adr/0005-term-representation.md#decision), points 6 and 7

<a id="opaque-handle"></a>**opaque handle** *(engineering)*: a value that identifies an object without revealing its layout, used only by passing it to accessor functions. In .NET terms it is like an `IntPtr` or `SafeHandle`.

- See also: [module interface](#module-interface)

<a id="polynomial-greatest-common-divisor"></a>**polynomial greatest common divisor** *(domain)*: the polynomial of highest degree that divides two given polynomials. Example: $\gcd(x^2 - 1,\ x^2 + 2x + 1) = x + 1$.

- See also: [REQ-010](requirements.md#req-010)

<a id="rational-function"></a>**rational function** *(domain)*: a quotient of two polynomials. Example: $\frac{x+1}{x-y}$. It is normalised when numerator and denominator have no common factor.

- See also: [RATFUN](#ratfun), [REQ-010](requirements.md#req-010)

<a id="ratfun"></a>**RATFUN** *(project)*: the [block](#block) kind of a coefficient that is a [rational function](#rational-function), as a numerator and a denominator polynomial. Example: the term $\frac{x+1}{x-y}\,f(z)$ has coefficient $\frac{x+1}{x-y}$.

- Kind of: [coefficient](#coefficient)
- Defined in: [ADR-0005](adr/0005-term-representation.md#decision), point 3
- After FORM: FORM holds such a coefficient in a reserved function, `PolyRatFun`. Tante makes it a coefficient kind, so code that walks factors never meets it.

<a id="reference"></a>**reference** (the reference) *(project)*: FORM, in the version fixed by [OPEN-D](requirements.md#open-d). It is a cross-check and defines what a FORM script means. It is not a specification of how Tante is built.

- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions)

<a id="reference-machine"></a>**reference machine** *(project)*: the machine every performance requirement is stated against, a GitHub-hosted standard Linux runner.

- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions)

<a id="renumbering"></a>**renumbering** *(project)*: one way to make the order of symbols depend on their names. Before each sort, symbols get new numbers in name order, so comparisons compare numbers directly. Example: for `z`, `a`, `m`, `a` becomes 0, `m` 1, `z` 2.

- See also: [table of ranks](#table-of-ranks)
- Defined in: [ADR-0005](adr/0005-term-representation.md#consequences)

<a id="ricci-flat"></a>**Ricci-flat** *(domain)*: a spacetime whose Ricci tensor vanishes, $R_{\mu\nu} = 0$. Example: the Schwarzschild metric, a corpus entry.

- See also: [REQ-070](requirements.md#req-070)

<a id="sanitizer"></a>**sanitizer** *(engineering)*: a compiler option that adds run-time checks for a class of bugs. Example: AddressSanitizer stops the program at an out-of-bounds access instead of letting it read garbage.

- See also: [static analysis](#static-analysis)
- Defined in: [ADR-0001](adr/0001-cpp20-cmake-vcpkg-guardrails.md#3-guardrails-for-a-maintainer-learning-c)

<a id="spike"></a>**spike** *(engineering)*: a short, throw-away experiment run to measure something before a decision. Its report lives in `docs/spikes/`. Its code stays on a branch that is never merged. Example: the [term-representation spike](spikes/2026-10-21-term-representation.md).

<a id="static-analysis"></a>**static analysis** *(engineering)*: checking source code for bugs without running it. Tante uses clang-tidy, which plays the part of the Roslyn analyzers in .NET.

- See also: [sanitizer](#sanitizer)
- Defined in: [ADR-0001](adr/0001-cpp20-cmake-vcpkg-guardrails.md#3-guardrails-for-a-maintainer-learning-c)

<a id="table-of-ranks"></a>**table of ranks** *(project)*: an array, indexed by symbol number, giving the position of each symbol's name in alphabetical order. Example: for `z`, `a`, `m` (numbers 0, 1, 2), the ranks are 2, 0, 1.

- See also: [renumbering](#renumbering)
- Defined in: [term-representation spike](spikes/2026-10-21-term-representation.md#terms-used)

<a id="term"></a>**term** *(domain)*: one product of a coefficient and factors (symbols, functions, later indices and tensors). Example: $\tfrac34 x^2 y\, f(a+b)$ is one term. $x + y$ is two. In Tante, a term is one contiguous sequence of [words](#word).

- See also: [expression](#expression), [block](#block)
- Not to be confused with: *term* in logic and rewriting, where it means any expression tree.
- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions) (under Expression)

<a id="type-length-value"></a>**type–length–value** *(engineering)*: an encoding in which each record begins with its kind and its length, so a reader can skip a record it does not understand. In .NET terms it is like a `BinaryReader` skipping a length-prefixed record.

- Defined in: [ADR-0005](adr/0005-term-representation.md)

<a id="vcpkg"></a>**vcpkg** *(engineering)*: Microsoft's open-source package manager for C and C++, the counterpart of NuGet. In manifest mode, a `vcpkg.json` lists the dependencies, as `PackageReference` does in a `.csproj`.

- Defined in: [ADR-0001](adr/0001-cpp20-cmake-vcpkg-guardrails.md)

<a id="view"></a>**view** *(project)*: a read-only cursor over the words of a term someone else owns. It neither owns nor copies the term. In .NET terms it is like reading a `ReadOnlySpan<byte>` with `Utf8JsonReader`.

- See also: [builder](#builder)
- Defined in: [ADR-0005](adr/0005-term-representation.md)

<a id="word"></a>**word** *(project)*: the 32-bit unsigned integer Tante's term encoding is counted in, C#'s `uint`. Every [field](#field-and-number) is one word.

- See also: [limb](#limb), [word size](#word-size)
- Not to be confused with: Win32 `WORD`, which is 16 bits.
- Defined in: [ADR-0005](adr/0005-term-representation.md#decision), point 2

<a id="word-size"></a>**word size** *(engineering)*: the natural integer width of a processor, 64 bits on today's desktop machines. Tante's [word](#word) is deliberately smaller: the spike measured 64-bit words to double the size of terms.

- Not to be confused with: [word](#word)
