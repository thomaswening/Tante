# Tante: Glossary

This glossary explains the terms a reader of Tante's documents may not know. Each entry is tagged by where the term comes from:

- *domain*: mathematics or physics, found in textbooks;
- *engineering*: software engineering, found in programming references;
- *project*: coined by Tante, or used by Tante in a sense of its own.

A document links a term on its first use, e.g. `[word](glossary.md#word)`. Anchors never change: a renamed term keeps its old anchor. Where [requirements.md](requirements.md) defines a term bindingly, the entry here is a one-line gloss and the requirements' wording governs.

Fixed labels after an entry relate it to others: **Kind of** names the broader term, **See also** related terms, **Not to be confused with** a look-alike with another meaning, **Defined in** the document whose wording governs, **After FORM** what Tante took from FORM and what it changed.

<a id="adapter"></a>**adapter** *(engineering)*: the one piece of code that connects an interface of Tante's own to an external library or a storage medium. Only an adapter includes that library's headers. In .NET terms it is the class that implements your own `IRepository` over Entity Framework. Example: `tante_poly_flint` implements Tante's polynomial interface with FLINT.

- See also: [include check](#include-check), [target](#target)
- Defined in: [ADR-0003](adr/0003-flint-gmp-arb-behind-own-interfaces.md), [ADR-0004](adr/0004-library-first-target-structure.md)

<a id="adr"></a>**ADR** (architecture decision record) *(engineering)*: a short document recording one significant, hard-to-reverse decision with its context, the alternatives and the consequences. Example: [ADR-0005](adr/0005-term-representation.md) records how terms are encoded. Each record is proposed in its own pull request and accepted by merging it.

- Defined in: [the ADR index](adr/README.md)

<a id="allocator-rounding"></a>**allocator rounding** *(engineering)*: the heap hands out memory in a fixed set of block sizes, so a request is rounded up to the next one. Example: a request for 20 bytes gets a 24- or 32-byte block, and the rest of the block is unusable.

- Defined in: [term-representation spike](spikes/2026-10-21-term-representation.md#terms-used)

<a id="api"></a>**API** (application programming interface) *(engineering)*: the functions and types a library offers to programs that use it. In .NET terms it is an assembly's public surface. Example: FORM is a program with no library API, so other programs can only run it and read its output.

- See also: [C interface](#c-interface)

<a id="arc42"></a>**arc42** *(engineering)*: a standard outline for architecture documents, a fixed list of sections: goals, constraints, context, solution strategy, building blocks, runtime view, cross-cutting concepts, decisions, risks (arc42.org). Example: [architecture.md](architecture.md) follows it.

- Defined in: [architecture.md](architecture.md)

<a id="ball-arithmetic"></a>**ball arithmetic** *(domain)*: arithmetic on intervals written as a midpoint and a radius, where every result is guaranteed to contain the exact value. Example: $\pi$ as $[3.14159265 \pm 4 \times 10^{-9}]$. Tante evaluates numerically through FLINT's ball arithmetic (formerly the Arb library).

- Defined in: [ADR-0003](adr/0003-flint-gmp-arb-behind-own-interfaces.md)

<a id="block"></a>**block** (`SYM`, `FUN`, `COEF`, …) *(project)*: one part of an encoded [term](#term), a sequence of [words](#word) that begins with its kind and its length. `SYM` holds symbols with their powers, `FUN` a function with its arguments, `COEF` or [`RATFUN`](#ratfun) the coefficient. Example: in $\tfrac34 x^2 y$, the block `SYM · 6 · x · 2 · y · 1` holds $x^2 y$.

- Kind of: [type–length–value](#type-length-value)
- Defined in: [ADR-0005](adr/0005-term-representation.md#decision)
- After FORM: FORM also encodes a term as a length, then subterms that each begin with a type and a length, then the coefficient ([prior-art.md §2.1](prior-art.md#21-form-streaming-terms-and-a-disk-sort-now-with-flint)). Tante keeps the idea. Its coefficient is a typed block of its own.

<a id="builder"></a>**builder** *(project)*: the class in the core that writes a new term word by word. It is the only way to create a term. In .NET terms it plays the part of `Utf8JsonWriter`.

- See also: [view](#view)
- Defined in: [ADR-0005](adr/0005-term-representation.md)

<a id="c-interface"></a>**C interface** *(project)*: the engine's interface in C, the header `tante.h`, for compiled modules and for bindings to other languages such as Python. It uses only C types and status codes, so any language that can call C can use it, as .NET does through P/Invoke.

- See also: [module interface](#module-interface), [FFI](#ffi)
- Defined in: [ADR-0001](adr/0001-cpp20-cmake-vcpkg-guardrails.md#4-the-c-interface-for-compiled-modules-and-bindings)

<a id="canonical-form"></a>**canonical form** *(domain)*: a way of writing objects such that two objects are equal exactly when they are written identically. Example: $(x+1)^2$ and $x^2 + 2x + 1$ are written as $x^2 + 2x + 1$.

- Defined in: [REQ-012](requirements.md#req-012)

<a id="canonical-order"></a>**canonical order** *(project)*: the one fixed order in which the engine writes the factors inside a term and the terms of an expression, so that equal expressions are stored and printed identically. Example: `y·x` and `x·y` are both stored as `x·y`.

- See also: [canonical form](#canonical-form), [table of ranks](#table-of-ranks), [renumbering](#renumbering)
- Defined in: [ADR-0005](adr/0005-term-representation.md); [term-representation spike](spikes/2026-10-21-term-representation.md#terms-used). Its rules are fixed by the record on canonical order.

<a id="checkpoint"></a>**checkpoint** *(project)*: a point in a calculation the user marks, at which the engine records the expression so it can be shown, explained or compared.

- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions)
- After FORM: FORM's checkpoints save a running calculation so that it can be restarted after a crash. FORM 5 deprecated them. [REQ-055](requirements.md#req-055)'s "checkpoint and resume" uses the word in FORM's sense.

<a id="coefficient"></a>**coefficient** *(domain)*: the factor of a term that is not a symbol or function. Example: in $\tfrac34 x^2 y$, the coefficient is $\tfrac34$. In Tante, a coefficient is either a rational number or a [rational function](#rational-function). It is always the last [block](#block) of a term.

- See also: [RATFUN](#ratfun), [term](#term)

<a id="corpus"></a>**the corpus** *(project)*: the set of calculations with independently known results that Tante's correctness is checked against. Example: the Schwarzschild metric is [Ricci-flat](#ricci-flat).

- Not to be confused with: [the FORM corpus](#form-corpus)
- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions); [REQ-070](requirements.md#req-070)

<a id="dollar-variable"></a>**dollar variable** *(engineering)*: a FORM variable whose name starts with `$` and whose value carries from one term to the next. Example: `$n = $n + 1;` counts the terms. When terms are processed by several threads, FORM runs such a step on one thread unless the script says how to combine the threads' values.

- After FORM: Tante requires a declared reduction (sum, maximum, minimum), or runs the step on one thread and says so ([REQ-074](requirements.md#req-074) (b)).
- Defined in: [architecture.md, section 7.1](architecture.md#71-form-concepts-req-025)

<a id="expression"></a>**expression** *(project)*: a mathematical object the engine holds and transforms: a sum of [terms](#term). Example: $x^2 + 2x + 1$ has three terms. In memory and on disk, an expression is a stream of terms behind a storage interface.

- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions); [ADR-0005](adr/0005-term-representation.md#decision), point 5

<a id="ffi"></a>**FFI** (foreign function interface) *(engineering)*: a way for code in one language to call functions written in another, usually through a C interface. In .NET terms it is P/Invoke (`[DllImport]`). Example: a Rust implementation of Tante would need FFI bindings to FLINT.

- See also: [C interface](#c-interface)

<a id="field-and-number"></a>**field and number** *(project)*: two different things in the term encoding. A *field* describes structure (a length, a count, a symbol number, a power) and normally takes one [word](#word). A *number* is a value such as a coefficient, stored as as many [limbs](#limb) as it needs. Example: $3/4$ takes one limb each for numerator and denominator.

- See also: [long form](#long-form)
- Defined in: [ADR-0005](adr/0005-term-representation.md)

<a id="flint"></a>**FLINT** (Fast Library for Number Theory) *(engineering)*: a C library for exact arithmetic. Tante uses it for polynomial multiplication, division, greatest common divisors and factorisation, behind an interface of its own. Licence: LGPL-3.0-or-later.

- See also: [GMP](#gmp), [MPFR](#mpfr), [ball arithmetic](#ball-arithmetic), [LGPL](#lgpl)
- Defined in: [ADR-0003](adr/0003-flint-gmp-arb-behind-own-interfaces.md#1-libraries-versions-and-licences)

<a id="flrw"></a>**FLRW** (Friedmann–Lemaître–Robertson–Walker) *(domain)*: the metric of a homogeneous, isotropic, expanding universe, with scale factor $a(t)$ and spatial curvature $k$:

```math
ds^2 = -dt^2 + a(t)^2 \left( \frac{dr^2}{1 - k r^2} + r^2 \, d\Omega^2 \right)
```

Example: its Kretschmann scalar is a corpus entry ([prior-art.md §5](prior-art.md#5-validation-corpus-catalogue)).

<a id="form-corpus"></a>**the FORM corpus** *(project)*: a set of publicly available FORM scripts with FORM's output for each, used to measure how well Tante runs FORM scripts.

- Not to be confused with: [the corpus](#corpus)
- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions); [REQ-021](requirements.md#req-021)

<a id="front-end"></a>**front end** *(project)*: a language the engine accepts instructions in. There are two, the native language and FORM scripts. Both are translated into the same internal representation.

- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions)

<a id="gcc"></a>**GCC** (GNU Compiler Collection) *(engineering)*: the standard C and C++ compiler on Linux. Tante is built with GCC or Clang on Linux and macOS, and with MSVC on Windows.

- See also: [MSVC](#msvc)

<a id="gmp"></a>**GMP** (GNU Multiple Precision Arithmetic Library) *(engineering)*: a C library for integers and rational numbers of any size, the counterpart of .NET's `System.Numerics.BigInteger`. FLINT is built on it. Licence: LGPL-3.0-or-later or GPL-2.0-or-later.

- See also: [FLINT](#flint), [limb](#limb)
- Defined in: [ADR-0003](adr/0003-flint-gmp-arb-behind-own-interfaces.md#1-libraries-versions-and-licences)

<a id="gpl"></a>**GPL** (GNU General Public License) *(engineering)*: a copyleft licence: whoever distributes the program or a modified version must provide its source under the same licence. Tante is licensed GPL-3.0-or-later.

- See also: [LGPL](#lgpl), [REQ-080](requirements.md#req-080)

<a id="hash-consing"></a>**hash-consing** *(engineering)*: keeping a hash table of every node built, so that each distinct subexpression exists once and is shared. *Cons* is Lisp's word for building a node. It is like `string.Intern` in .NET. Tante does not use it.

- Defined in: [term-representation spike](spikes/2026-10-21-term-representation.md#terms-used)

<a id="ibp"></a>**IBP** (integration by parts) *(domain)*: identities between Feynman integrals, obtained because the integral of a total derivative vanishes in dimensional regularisation. Solving them reduces many integrals to a few master integrals. It produces large linear systems with rational-function coefficients. Example: FORM 5's `mbox1l` benchmark, a corpus entry.

- See also: [rational function](#rational-function)

<a id="include-check"></a>**include check** *(engineering)*: a script (`tools/check_architecture.py`) that fails CI on any `#include` a directory may not use. It is needed because vcpkg installs all libraries' headers in one folder.

- Defined in: [ADR-0004](adr/0004-library-first-target-structure.md)

<a id="inlining"></a>**inlining** *(engineering)*: the compiler copies a function's body into its caller, so no call happens at run time. It is impossible across a shared-library boundary, since neither side's compiler sees the other side's code.

- Defined in: [term-representation spike](spikes/2026-10-21-term-representation.md#terms-used)

<a id="laurent-series"></a>**Laurent series** *(domain)*: a power series that may also contain negative powers. Example: $\frac{1}{\sin x} = x^{-1} + \frac{x}{6} + \frac{7x^3}{360} + \dots$ It is the reason powers in a term are signed.

- See also: [REQ-093](requirements.md#req-093)

<a id="lgpl"></a>**LGPL** (GNU Lesser General Public License) *(engineering)*: a licence for libraries. A program that uses the library may have any licence compatible with it. Changes to the library itself stay under the LGPL. FLINT, GMP and MPFR are LGPL.

- Kind of: [GPL](#gpl) (a weaker form of it)
- See also: [FLINT](#flint)

<a id="limb"></a>**limb** *(engineering)*: one word-sized digit of a large number. Example: a 96-digit integer takes ten 32-bit limbs.

- See also: [word](#word)
- Defined in: [term-representation spike](spikes/2026-10-21-term-representation.md#terms-used); [ADR-0005](adr/0005-term-representation.md)

<a id="long-form"></a>**long form** *(project)*: how a [field](#field-and-number) too large for one word is written: a marker word, then two words holding a 64-bit value. Example: a power of $2^{40}$. No field ever needs more, since no memory or disk holds $2^{63}$ words.

- Defined in: [ADR-0005](adr/0005-term-representation.md#decision), point 2

<a id="mhv"></a>**MHV** (maximally helicity violating) *(domain)*: a gluon scattering amplitude in which exactly two gluons have negative helicity and all others positive, the most unequal combination whose tree amplitude is not zero. The Parke–Taylor formula gives it in one line:

```math
A_n = \frac{\langle i j \rangle^4}{\langle 1 2 \rangle \langle 2 3 \rangle \cdots \langle n 1 \rangle}
```

Example: a corpus entry ([REQ-070](requirements.md#req-070)).

<a id="module"></a>**module** *(project)*: a unit of capability (new kinds of object, rules, functions, conventions) added to the engine through its public extension interface. The engine's own domains beyond the core are modules too.

- See also: [module interface](#module-interface)
- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions)
- After FORM: in a FORM program, a module is the part that ends with a `.sort` statement. Each module runs over all terms before the next one starts.

<a id="module-interface"></a>**module interface** (C module interface) *(project)*: the one C interface through which every [module](#module), built-in or third-party, connects to the engine. Modules see terms through it only as [opaque handles](#opaque-handle). Built-in modules compile against a second implementation that calls the core directly.

- Defined in: [ADR-0004](adr/0004-library-first-target-structure.md); [ADR-0005](adr/0005-term-representation.md#decision), points 6 and 7

<a id="module-wrapper"></a>**module wrapper** *(project)*: the C++ header `tante/module.hpp` that module authors write against. It gives them [RAII](#raii) handles and exceptions over the C module interface. It has two implementations: one calls the C module interface, the other calls the core directly and is used by the built-in modules for speed.

- See also: [module interface](#module-interface)
- Defined in: [ADR-0004](adr/0004-library-first-target-structure.md), [ADR-0005](adr/0005-term-representation.md)

<a id="mpfr"></a>**MPFR** *(engineering)*: a C library for floating-point numbers of chosen precision with correct rounding, built on GMP. It is a dependency of FLINT. Tante does not call it directly. Licence: LGPL-3.0-or-later.

- See also: [GMP](#gmp), [FLINT](#flint)
- Defined in: [ADR-0003](adr/0003-flint-gmp-arb-behind-own-interfaces.md#1-libraries-versions-and-licences)

<a id="mpi"></a>**MPI** (Message Passing Interface) *(engineering)*: a standard for programs that run on many machines at once and exchange messages. FORM's distributed version, ParFORM, used it. FORM 5 deprecated ParFORM in favour of threads on one machine. Tante's first release leaves MPI out.

- See also: [TFORM](#tform)

<a id="msvc"></a>**MSVC** (Microsoft Visual C++) *(engineering)*: Microsoft's C and C++ compiler, `cl.exe`, the one Visual Studio uses. Tante is built with it on Windows.

- See also: [GCC](#gcc)

<a id="mzv"></a>**MZV** (multiple zeta value) *(domain)*: a number defined by a nested sum, with $s_1 \ge 2$:

```math
\zeta(s_1, \dots, s_k) = \sum_{n_1 > n_2 > \dots > n_k \ge 1} \frac{1}{n_1^{s_1} \cdots n_k^{s_k}}
```

Example: $\zeta(2) = \pi^2/6$. MZVs appear in Feynman integrals. FORM computed a large database of the relations between them, the MZV data mine.

<a id="opaque-handle"></a>**opaque handle** *(engineering)*: a value that identifies an object without revealing its layout, used only by passing it to accessor functions. In .NET terms it is like an `IntPtr` or `SafeHandle`.

- See also: [module interface](#module-interface)

<a id="polynomial-greatest-common-divisor"></a>**polynomial greatest common divisor** (GCD) *(domain)*: the polynomial of highest degree that divides two given polynomials. Example: $\gcd(x^2 - 1,\ x^2 + 2x + 1) = x + 1$.

- See also: [REQ-010](requirements.md#req-010)

<a id="preprocessor"></a>**preprocessor** *(engineering)*: a stage that rewrites the text of a script before it is parsed. FORM's preprocessor has loops, conditions and procedures, which FORM scripts use for program logic. Example: `#do i = 1, 3` … `#enddo` repeats the lines between three times, with `i` replaced by 1, 2 and 3.

- Not to be confused with: the C preprocessor, which Tante's own C++ code uses for `#include`.
- Defined in: [architecture.md, section 7.1](architecture.md#71-form-concepts-req-025)

<a id="qcd"></a>**QCD** (quantum chromodynamics) *(domain)*: the quantum field theory of the strong interaction between quarks and gluons, with colour group SU(3).

- Kind of: [QFT](#qft)

<a id="qed"></a>**QED** (quantum electrodynamics) *(domain)*: the quantum field theory of electrons and photons. Example: the corpus entry $e^+e^- \to \mu^+\mu^-$.

- Kind of: [QFT](#qft)

<a id="qft"></a>**QFT** (quantum field theory) *(domain)*: the framework of particle physics, in which particles are excitations of fields. Most of FORM's large calculations are QFT calculations.

<a id="quality-goal"></a>**quality goal** *(engineering)*: how well a system must do all of its work, as opposed to what it does. Quality goals cut across every feature. Tante ranks six, and the higher one wins when two conflict. Example: determinism ranks above speed, so terms from several threads are merged in a fixed order.

- Defined in: [architecture.md, section 1](architecture.md#1-goals)

<a id="raii"></a>**RAII** (resource acquisition is initialisation) *(engineering)*: the C++ rule that an object releases its resources in its destructor, which runs automatically when the object goes out of scope. In .NET terms it is like `using` with `IDisposable`, except that nothing has to be written at the point of use.

- Defined in: [ADR-0001](adr/0001-cpp20-cmake-vcpkg-guardrails.md#3-guardrails-for-a-maintainer-learning-c)

<a id="ratfun"></a>**RATFUN** *(project)*: the [block](#block) kind of a coefficient that is a [rational function](#rational-function), as a numerator and a denominator polynomial. Example: the term $\frac{x+1}{x-y}\,f(z)$ has coefficient $\frac{x+1}{x-y}$.

- Kind of: [coefficient](#coefficient)
- Defined in: [ADR-0005](adr/0005-term-representation.md#decision), point 3
- After FORM: FORM holds such a coefficient in a reserved function, `PolyRatFun`. Tante makes it a coefficient kind, so code that walks factors never meets it.

<a id="rational-function"></a>**rational function** *(domain)*: a quotient of two polynomials. Example: $\frac{x+1}{x-y}$. It is normalised when numerator and denominator have no common factor.

- See also: [RATFUN](#ratfun), [REQ-010](requirements.md#req-010)

<a id="reference"></a>**the reference** (FORM) *(project)*: FORM, in the version fixed by [OPEN-D](requirements.md#open-d). It is a cross-check and defines what a FORM script means. It is not a specification of how Tante is built.

- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions)

<a id="reference-machine"></a>**reference machine** *(project)*: the machine every performance requirement is stated against, a GitHub-hosted standard Linux runner.

- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions)

<a id="renumbering"></a>**renumbering** *(project)*: one way to make the order of symbols depend on their names. Before each sort, symbols get new numbers in name order, so comparisons compare numbers directly. Example: for `z`, `a`, `m`, `a` becomes 0, `m` 1, `z` 2.

- See also: [table of ranks](#table-of-ranks)
- Defined in: [ADR-0005](adr/0005-term-representation.md#consequences)

<a id="ricci-flat"></a>**Ricci-flat** *(domain)*: a spacetime whose Ricci tensor vanishes, $R_{\mu\nu} = 0$. Example: the Schwarzschild metric, a corpus entry.

- See also: [REQ-070](requirements.md#req-070)

<a id="runtime-view"></a>**runtime view** *(engineering)*: a picture of what the parts of a system do, in order, while it runs, usually a sequence diagram. The building-block diagram shows which parts exist. The runtime view shows how they call each other. Example: [section 6 of architecture.md](architecture.md#6-runtime-view) follows one script from the command-line tool to the store.

- Kind of: a section of [arc42](#arc42)
- Defined in: [architecture.md](architecture.md)

<a id="sanitizer"></a>**sanitizer** *(engineering)*: a compiler option that adds run-time checks for a class of bugs. Example: AddressSanitizer stops the program at an out-of-bounds access instead of letting it read garbage.

- See also: [static analysis](#static-analysis)
- Defined in: [ADR-0001](adr/0001-cpp20-cmake-vcpkg-guardrails.md#3-guardrails-for-a-maintainer-learning-c)

<a id="shared-library"></a>**shared library** *(engineering)*: machine code loaded at run time, a `.dll`, `.so` or `.dylib`, like an assembly. Unlike an assembly it has no compiler-neutral metadata, so C++ types work across it only with the compiler that built it. Plain C functions and types work with any compiler. Example: a third-party compiled module is a shared library.

- Not to be confused with: [static library](#static-library)
- Defined in: [ADR-0004](adr/0004-library-first-target-structure.md)

<a id="spike"></a>**spike** *(engineering)*: a short, throw-away experiment run to measure something before a decision. Its report lives in `docs/spikes/`. Its code stays on a branch that is never merged. Example: the [term-representation spike](spikes/2026-10-21-term-representation.md).

<a id="stage"></a>**stage** *(project)*: a section of a calculation that ends when the expression is sorted and equal terms are merged. Example: in a FORM script, every statement before the first `.sort` belongs to the first stage.

- After FORM: the idea and FORM's `.sort`. Where a stage ends in the native language is for the record on the execution model.
- Not to be confused with: the six stages of [OPEN-K](requirements.md#open-k), the order in which Tante's capabilities are built. Stage 5 there is the FORM front end.
- See also: [store](#store)
- Defined in: [architecture.md, section 6](architecture.md#6-runtime-view)

<a id="static-analysis"></a>**static analysis** *(engineering)*: checking source code for bugs without running it. Tante uses clang-tidy, which plays the part of the Roslyn analyzers in .NET.

- See also: [sanitizer](#sanitizer)
- Defined in: [ADR-0001](adr/0001-cpp20-cmake-vcpkg-guardrails.md#3-guardrails-for-a-maintainer-learning-c)

<a id="static-library"></a>**static library** *(engineering)*: machine code that the linker copies into the executable at build time, a `.lib` or `.a`, as if every referenced assembly were merged into one executable. Example: `tante_core` is a static library compiled into the command-line tool `tante`.

- Not to be confused with: [shared library](#shared-library)
- Defined in: [ADR-0004](adr/0004-library-first-target-structure.md)

<a id="store"></a>**store** *(project)*: where an expression's terms are kept, behind an interface with three phases: append terms, sort them while merging equal terms, read them in order. It offers no random access and no count of terms, so it can keep the terms in memory or on disk. Example: `tante_store_memory` comes first; a store on disk is added for [REQ-054](requirements.md#req-054).

- See also: [expression](#expression), [stage](#stage)
- Defined in: [ADR-0005](adr/0005-term-representation.md)

<a id="table-of-ranks"></a>**table of ranks** *(project)*: an array, indexed by symbol number, giving the position of each symbol's name in alphabetical order. Example: for `z`, `a`, `m` (numbers 0, 1, 2), the ranks are 2, 0, 1.

- See also: [renumbering](#renumbering)
- Defined in: [term-representation spike](spikes/2026-10-21-term-representation.md#terms-used); [ADR-0005](adr/0005-term-representation.md)

<a id="target"></a>**target** *(engineering)*: a unit of a CMake build, a library or an executable with its own sources and dependencies, like a project in a .NET solution. Example: `tante_core` is a target, and the command-line tool `tante` is another.

- See also: [static library](#static-library)
- Defined in: [ADR-0004](adr/0004-library-first-target-structure.md)

<a id="term"></a>**term** *(domain)*: one product of a coefficient and factors (symbols, functions, later indices and tensors). Example: $\tfrac34 x^2 y\, f(a+b)$ is one term. $x + y$ is two. In Tante, a term is one contiguous sequence of [words](#word).

- See also: [expression](#expression), [block](#block)
- Not to be confused with: *term* in logic and rewriting, where it means any expression tree.
- Defined in: [requirements.md, Requirement conventions](requirements.md#requirement-conventions) (under Expression)

<a id="tform"></a>**TFORM** *(engineering)*: the multithreaded version of FORM, which runs one calculation on several cores of one machine (Tentyukov and Vermaseren, CPC 181 (2010) 1419).

- See also: [the reference](#reference), [MPI](#mpi)

<a id="type-length-value"></a>**type–length–value** *(engineering)*: an encoding in which each record begins with its kind and its length, so a reader can skip a record it does not understand. In .NET terms it is like a `BinaryReader` skipping a length-prefixed record.

- Defined in: [ADR-0005](adr/0005-term-representation.md)

<a id="vcpkg"></a>**vcpkg** *(engineering)*: Microsoft's open-source package manager for C and C++, the counterpart of NuGet. In manifest mode, a `vcpkg.json` lists the dependencies, as `PackageReference` does in a `.csproj`.

- Defined in: [ADR-0001](adr/0001-cpp20-cmake-vcpkg-guardrails.md)

<a id="view"></a>**view** *(project)*: a read-only cursor over the words of a term someone else owns. It neither owns nor copies the term. In .NET terms it is like reading a `ReadOnlySpan<byte>` with `Utf8JsonReader`.

- See also: [builder](#builder)
- Defined in: [ADR-0005](adr/0005-term-representation.md)

<a id="wildcard"></a>**wildcard** *(engineering)*: a placeholder in a pattern that matches any object of its kind and names it for the replacement. Example: in FORM's `id f(x?) = x^2;` the wildcard `x?` matches any argument, so $f(a)$ becomes $a^2$.

- Defined in: [REQ-011](requirements.md#req-011)

<a id="word"></a>**word** *(project)*: the 32-bit unsigned integer Tante's term encoding is counted in, C#'s `uint`. Every [field](#field-and-number) is one word.

- See also: [limb](#limb), [word size](#word-size)
- Not to be confused with: Win32 `WORD`, which is 16 bits.
- Defined in: [ADR-0005](adr/0005-term-representation.md#decision), point 2
- After FORM: FORM also counts terms in words. Its MaxTermSize caps one term at 40 000 words by default ([prior-art.md §2.1](prior-art.md#21-form-streaming-terms-and-a-disk-sort-now-with-flint)).

<a id="word-size"></a>**word size** *(engineering)*: the natural integer width of a processor, 64 bits on today's desktop machines. Tante's [word](#word) is deliberately smaller: the spike measured 64-bit words to double the size of terms.

- Not to be confused with: [word](#word)
