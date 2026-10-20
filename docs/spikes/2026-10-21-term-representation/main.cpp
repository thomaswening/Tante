// Spike for ADR-0005 (docs/spikes/2026-10-21-term-representation.md).
//
// Measures, for generated workloads:
//   1. bytes per term of a flat, pointer-free encoding with 32-bit and 64-bit words,
//      and of a tree of reference-counted nodes;
//   2. the time to sort an expression and add the coefficients of equal terms, in each
//      representation, with symbols compared by number or through a table of ranks by name;
//   3. the per-term cost of a module function called directly and through the C interface.
//
// Usage: spike <output-directory> [quick]
// Writes results.txt (must be byte-identical on all systems), timings.txt, memory.txt, info.txt.
// Exits non-zero if the representations disagree on any result.

#include "module_logic.hpp"
#include "spike_module.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <malloc.h>
#elif defined(__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

// ---------------------------------------------------------------------------------------
// Heap accounting: bytes in use, as the allocator rounds them, while counting is on.

namespace {
bool g_counting = false;
std::size_t g_live_bytes = 0;
std::size_t g_allocations = 0;

#ifndef SPIKE_SANITIZE
std::size_t usable_size(void* p) noexcept
{
#if defined(_WIN32)
    return _msize(p);
#elif defined(__APPLE__)
    return malloc_size(p);
#else
    return malloc_usable_size(p);
#endif
}
#endif
} // namespace

#ifndef SPIKE_SANITIZE
void* operator new(std::size_t n)
{
    void* p = std::malloc(n == 0 ? 1 : n);
    if (p == nullptr) {
        throw std::bad_alloc();
    }
    if (g_counting) {
        g_live_bytes += usable_size(p);
        ++g_allocations;
    }
    return p;
}
void operator delete(void* p) noexcept
{
    if (p != nullptr) {
        if (g_counting) {
            g_live_bytes -= usable_size(p);
        }
        std::free(p);
    }
}
void operator delete(void* p, std::size_t) noexcept { operator delete(p); }
void* operator new[](std::size_t n) { return operator new(n); }
void operator delete[](void* p) noexcept { operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { operator delete(p); }
#endif

namespace {

// ---------------------------------------------------------------------------------------
// Deterministic generation: SplitMix64, identical on every system.

struct SplitMix64 {
    std::uint64_t state;
    std::uint64_t next()
    {
        std::uint64_t z = (state += 0x9E3779B97F4A7C15ull);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    }
    std::uint32_t below(std::uint32_t n) { return static_cast<std::uint32_t>(next() % n); }
};

// Symbols are numbered in creation order. Canonical order (REQ-012) must not depend on
// that order, so symbols are compared through their rank by name. The spike stands in
// a fixed permutation for the ranks.
constexpr std::uint32_t kSymbols = 64;
std::array<std::uint32_t, kSymbols> g_rank{};

void init_ranks()
{
    for (std::uint32_t i = 0; i < kSymbols; ++i) {
        g_rank[i] = (i * 37u + 11u) % kSymbols;
    }
}

enum class Order { Id, Rank };

template <Order O>
inline std::uint32_t key(std::uint32_t symbol)
{
    if constexpr (O == Order::Id) {
        return symbol;
    } else {
        return g_rank[symbol];
    }
}

using SymPow = std::pair<std::uint32_t, std::uint32_t>;

// A term as the generator produces it, before encoding.
struct ATerm;
struct AFun {
    std::uint32_t id = 0;
    std::vector<std::vector<ATerm>> args;
};
struct ATerm {
    std::vector<SymPow> syms;
    std::vector<AFun> funs;
    std::vector<std::uint64_t> coef; // positive integer, 64-bit limbs, little-endian, no leading zero limb
};

enum class Kind { Poly4, Poly20, BigCoef, Nested };

void generate(Kind kind, SplitMix64& r, ATerm& t)
{
    t.syms.clear();
    t.funs.clear();
    t.coef.clear();
    switch (kind) {
    case Kind::Poly4:
    case Kind::BigCoef:
        for (std::uint32_t s = 0; s < 4; ++s) {
            const std::uint32_t e = r.below(32);
            if (e != 0) {
                t.syms.emplace_back(s, e);
            }
        }
        if (kind == Kind::Poly4) {
            t.coef.push_back(1 + r.below(1u << 20));
        } else {
            for (int i = 0; i < 4; ++i) {
                t.coef.push_back(r.next());
            }
            t.coef.push_back(r.next() | 1u); // about 96 decimal digits
        }
        break;
    case Kind::Poly20: {
        std::array<std::uint32_t, 40> ids{};
        for (std::uint32_t i = 0; i < 40; ++i) {
            ids[i] = i;
        }
        for (std::uint32_t i = 0; i < 20; ++i) {
            const std::uint32_t j = i + r.below(40 - i);
            std::swap(ids[i], ids[j]);
            t.syms.emplace_back(ids[i], 1 + r.below(3));
        }
        t.coef.push_back(1 + r.below(1u << 20));
        break;
    }
    case Kind::Nested: {
        for (std::uint32_t s = 0; s < 4; ++s) {
            const std::uint32_t e = r.below(8);
            if (e != 0) {
                t.syms.emplace_back(s, e);
            }
        }
        AFun f;
        f.id = r.below(2);
        std::vector<ATerm> arg(2);
        const std::uint32_t a = 4 + r.below(4);
        const std::uint32_t b = 4 + (a - 4 + 1 + r.below(3)) % 4;
        arg[0].syms.emplace_back(a, 1 + r.below(3));
        arg[0].coef.push_back(1 + r.below(5));
        arg[1].syms.emplace_back(b, 1 + r.below(3));
        arg[1].coef.push_back(1 + r.below(5));
        f.args.push_back(std::move(arg));
        t.funs.push_back(std::move(f));
        t.coef.push_back(1 + r.below(1u << 20));
        break;
    }
    }
}

// ---------------------------------------------------------------------------------------
// The flat encoding. A term is
//   [length] [SYM block]? [FUN block]* [COEF block]
// where every block starts with its kind and its own length in words:
//   SYM:  [kSym, len, symbol, power, symbol, power, ...]   symbols in canonical order
//   FUN:  [kFun, len, function, ARG...]
//   ARG:  [kArg, len, term, term, ...]                     an argument is an expression
//   COEF: [kCoef, len, numerator-limb-count, numerator limbs..., denominator limbs...]
// Numbers are little-endian, word-sized limbs without leading zero limbs.

constexpr std::uint32_t kSym = 1;
constexpr std::uint32_t kFun = 2;
constexpr std::uint32_t kArg = 3;
constexpr std::uint32_t kCoef = 4;

template <class W>
std::size_t append_limbs(std::vector<W>& out, const std::uint64_t* v, std::size_t n)
{
    const std::size_t start = out.size();
    for (std::size_t i = 0; i < n; ++i) {
        if constexpr (sizeof(W) == 8) {
            out.push_back(v[i]);
        } else {
            out.push_back(static_cast<W>(v[i]));
            out.push_back(static_cast<W>(v[i] >> 32));
        }
    }
    while (out.size() - start > 1 && out.back() == 0) {
        out.pop_back();
    }
    return out.size() - start;
}

template <Order O>
std::vector<SymPow> sorted_syms(const std::vector<SymPow>& syms)
{
    std::vector<SymPow> s = syms;
    std::sort(s.begin(), s.end(), [](const SymPow& x, const SymPow& y) { return key<O>(x.first) < key<O>(y.first); });
    return s;
}

// Terms of an argument each have one distinct symbol in this spike, so their canonical
// order is that of their first symbol.
template <Order O>
std::vector<const ATerm*> sorted_arg(const std::vector<ATerm>& arg)
{
    std::vector<const ATerm*> v;
    for (const ATerm& x : arg) {
        v.push_back(&x);
    }
    std::sort(v.begin(), v.end(),
              [](const ATerm* x, const ATerm* y) { return key<O>(x->syms.front().first) < key<O>(y->syms.front().first); });
    return v;
}

template <class W, Order O>
void encode(const ATerm& t, std::vector<W>& out)
{
    const std::size_t at = out.size();
    out.push_back(0);
    if (!t.syms.empty()) {
        const std::vector<SymPow> s = sorted_syms<O>(t.syms);
        out.push_back(kSym);
        out.push_back(static_cast<W>(2 + 2 * s.size()));
        for (const auto& [sym, pow] : s) {
            out.push_back(sym);
            out.push_back(pow);
        }
    }
    for (const AFun& f : t.funs) {
        const std::size_t fa = out.size();
        out.push_back(kFun);
        out.push_back(0);
        out.push_back(f.id);
        for (const std::vector<ATerm>& arg : f.args) {
            const std::size_t aa = out.size();
            out.push_back(kArg);
            out.push_back(0);
            for (const ATerm* x : sorted_arg<O>(arg)) {
                encode<W, O>(*x, out);
            }
            out[aa + 1] = static_cast<W>(out.size() - aa);
        }
        out[fa + 1] = static_cast<W>(out.size() - fa);
    }
    const std::size_t ca = out.size();
    out.push_back(kCoef);
    out.push_back(0);
    out.push_back(0);
    out[ca + 2] = static_cast<W>(append_limbs<W>(out, t.coef.data(), t.coef.size()));
    const std::uint64_t one = 1;
    append_limbs<W>(out, &one, 1);
    out[ca + 1] = static_cast<W>(out.size() - ca);
    out[at] = static_cast<W>(out.size() - at);
}

template <class W>
inline std::size_t len(W w)
{
    return static_cast<std::size_t>(w);
}

template <class W>
inline int cmp_limbs(const W* a, std::size_t na, const W* b, std::size_t nb)
{
    if (na != nb) {
        return na < nb ? -1 : 1;
    }
    for (std::size_t i = na; i-- > 0;) {
        if (a[i] != b[i]) {
            return a[i] < b[i] ? -1 : 1;
        }
    }
    return 0;
}

template <class W, Order O>
int cmp_term(const W* a, const W* b, bool with_coef);

template <class W, Order O>
int cmp_fun(const W* fa, const W* fb)
{
    if (fa[2] != fb[2]) {
        return fa[2] < fb[2] ? -1 : 1;
    }
    const W* pa = fa + 3;
    const W* ea = fa + len(fa[1]);
    const W* pb = fb + 3;
    const W* eb = fb + len(fb[1]);
    while (pa < ea && pb < eb) {
        const W* ta = pa + 2;
        const W* xa = pa + len(pa[1]);
        const W* tb = pb + 2;
        const W* xb = pb + len(pb[1]);
        while (ta < xa && tb < xb) {
            const int c = cmp_term<W, O>(ta, tb, true);
            if (c != 0) {
                return c;
            }
            ta += len(ta[0]);
            tb += len(tb[0]);
        }
        if (ta < xa) {
            return 1;
        }
        if (tb < xb) {
            return -1;
        }
        pa = xa;
        pb = xb;
    }
    if (pa < ea) {
        return 1;
    }
    if (pb < eb) {
        return -1;
    }
    return 0;
}

// The spike's stand-in for canonical order (ADR-0006 decides the real one): symbols by
// key, then higher power first, fewer symbols first; then functions; then, inside
// arguments only, the coefficient by value.
template <class W, Order O>
int cmp_term(const W* a, const W* b, bool with_coef)
{
    const W* pa = a + 1;
    const W* pb = b + 1;
    const W* sa = pa;
    const W* sb = pb;
    std::size_t na = 0;
    std::size_t nb = 0;
    if (pa[0] == kSym) {
        sa = pa + 2;
        na = (len(pa[1]) - 2) / 2;
        pa += len(pa[1]);
    }
    if (pb[0] == kSym) {
        sb = pb + 2;
        nb = (len(pb[1]) - 2) / 2;
        pb += len(pb[1]);
    }
    const std::size_t n = std::min(na, nb);
    for (std::size_t i = 0; i < n; ++i) {
        const std::uint32_t ka = key<O>(static_cast<std::uint32_t>(sa[2 * i]));
        const std::uint32_t kb = key<O>(static_cast<std::uint32_t>(sb[2 * i]));
        if (ka != kb) {
            return ka < kb ? -1 : 1;
        }
        if (sa[2 * i + 1] != sb[2 * i + 1]) {
            return sa[2 * i + 1] > sb[2 * i + 1] ? -1 : 1;
        }
    }
    if (na != nb) {
        return na < nb ? -1 : 1;
    }
    while (pa[0] == kFun && pb[0] == kFun) {
        const int c = cmp_fun<W, O>(pa, pb);
        if (c != 0) {
            return c;
        }
        pa += len(pa[1]);
        pb += len(pb[1]);
    }
    if (pa[0] == kFun) {
        return 1;
    }
    if (pb[0] == kFun) {
        return -1;
    }
    if (!with_coef) {
        return 0;
    }
    const std::size_t nna = len(pa[2]);
    const std::size_t nnb = len(pb[2]);
    const int c = cmp_limbs(pa + 3, nna, pb + 3, nnb);
    if (c != 0) {
        return c;
    }
    return cmp_limbs(pa + 3 + nna, len(pa[1]) - 3 - nna, pb + 3 + nnb, len(pb[1]) - 3 - nnb);
}

template <class W>
const W* coef_of(const W* t)
{
    const W* p = t + 1;
    while (p[0] != kCoef) {
        p += len(p[1]);
    }
    return p;
}

template <class W>
void add_limbs(std::vector<W>& s, const W* b, std::size_t nb)
{
    if (s.size() < nb) {
        s.resize(nb, 0);
    }
    W carry = 0;
    for (std::size_t i = 0; i < s.size(); ++i) {
        const W bi = i < nb ? b[i] : W{0};
        const W x = static_cast<W>(s[i] + bi);
        const W c1 = static_cast<W>(x < bi);
        const W y = static_cast<W>(x + carry);
        const W c2 = static_cast<W>(y < carry);
        s[i] = y;
        carry = static_cast<W>(c1 | c2);
        if (i + 1 >= nb && carry == 0) {
            return;
        }
    }
    if (carry != 0) {
        s.push_back(1);
    }
}

template <class W>
struct FlatExpr {
    std::vector<W> words;
    std::vector<std::size_t> offsets;
};

template <class W, Order O>
FlatExpr<W> build_flat(Kind kind, std::size_t n, std::uint64_t seed)
{
    FlatExpr<W> e;
    SplitMix64 r{seed};
    ATerm t;
    e.offsets.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        generate(kind, r, t);
        e.offsets.push_back(e.words.size());
        encode<W, O>(t, e.words);
    }
    e.words.shrink_to_fit();
    return e;
}

// Sort the terms (through an array of offsets, as FORM sorts pointers to terms), then
// write one term per run of equal terms with the coefficients added.
template <class W, Order O>
FlatExpr<W> sort_merge(const FlatExpr<W>& in)
{
    std::vector<std::size_t> idx = in.offsets;
    const W* base = in.words.data();
    std::sort(idx.begin(), idx.end(),
              [base](std::size_t x, std::size_t y) { return cmp_term<W, O>(base + x, base + y, false) < 0; });
    FlatExpr<W> out;
    out.words.reserve(in.words.size());
    out.offsets.reserve(idx.size());
    std::vector<W> sum;
    for (std::size_t i = 0; i < idx.size();) {
        const W* t = base + idx[i];
        const W* c = coef_of(t);
        sum.assign(c + 3, c + 3 + len(c[2]));
        std::size_t j = i + 1;
        while (j < idx.size() && cmp_term<W, O>(t, base + idx[j], false) == 0) {
            const W* cj = coef_of(base + idx[j]);
            add_limbs(sum, cj + 3, len(cj[2]));
            ++j;
        }
        const std::size_t at = out.words.size();
        out.offsets.push_back(at);
        out.words.insert(out.words.end(), t, c);
        const std::size_t ca = out.words.size();
        out.words.push_back(kCoef);
        out.words.push_back(0);
        out.words.push_back(static_cast<W>(sum.size()));
        out.words.insert(out.words.end(), sum.begin(), sum.end());
        out.words.push_back(1);
        out.words[ca + 1] = static_cast<W>(out.words.size() - ca);
        out.words[at] = static_cast<W>(out.words.size() - at);
        i = j;
    }
    return out;
}

// ---------------------------------------------------------------------------------------
// The tree baseline: reference-counted nodes, as in GiNaC or SymEngine (without their
// hash-consing).

struct TTerm;
using TPtr = std::shared_ptr<const TTerm>;
struct TFun {
    std::uint32_t id = 0;
    std::vector<std::vector<TPtr>> args;
};
struct TTerm {
    std::vector<SymPow> syms;
    std::vector<TFun> funs;
    std::vector<std::uint64_t> num;
    std::vector<std::uint64_t> den;
};

template <Order O>
TPtr to_tree(const ATerm& a)
{
    auto t = std::make_shared<TTerm>();
    t->syms = sorted_syms<O>(a.syms);
    t->funs.reserve(a.funs.size());
    for (const AFun& f : a.funs) {
        TFun tf;
        tf.id = f.id;
        tf.args.reserve(f.args.size());
        for (const std::vector<ATerm>& arg : f.args) {
            std::vector<TPtr> v;
            v.reserve(arg.size());
            for (const ATerm* x : sorted_arg<O>(arg)) {
                v.push_back(to_tree<O>(*x));
            }
            tf.args.push_back(std::move(v));
        }
        t->funs.push_back(std::move(tf));
    }
    t->num = a.coef;
    t->den = {1};
    return t;
}

int cmp_vec(const std::vector<std::uint64_t>& a, const std::vector<std::uint64_t>& b)
{
    return cmp_limbs(a.data(), a.size(), b.data(), b.size());
}

template <Order O>
int cmp_tree(const TTerm& a, const TTerm& b, bool with_coef);

template <Order O>
int cmp_tfun(const TFun& a, const TFun& b)
{
    if (a.id != b.id) {
        return a.id < b.id ? -1 : 1;
    }
    const std::size_t n = std::min(a.args.size(), b.args.size());
    for (std::size_t i = 0; i < n; ++i) {
        const std::vector<TPtr>& x = a.args[i];
        const std::vector<TPtr>& y = b.args[i];
        const std::size_t m = std::min(x.size(), y.size());
        for (std::size_t j = 0; j < m; ++j) {
            const int c = cmp_tree<O>(*x[j], *y[j], true);
            if (c != 0) {
                return c;
            }
        }
        if (x.size() != y.size()) {
            return x.size() < y.size() ? -1 : 1;
        }
    }
    if (a.args.size() != b.args.size()) {
        return a.args.size() < b.args.size() ? -1 : 1;
    }
    return 0;
}

template <Order O>
int cmp_tree(const TTerm& a, const TTerm& b, bool with_coef)
{
    const std::size_t n = std::min(a.syms.size(), b.syms.size());
    for (std::size_t i = 0; i < n; ++i) {
        const std::uint32_t ka = key<O>(a.syms[i].first);
        const std::uint32_t kb = key<O>(b.syms[i].first);
        if (ka != kb) {
            return ka < kb ? -1 : 1;
        }
        if (a.syms[i].second != b.syms[i].second) {
            return a.syms[i].second > b.syms[i].second ? -1 : 1;
        }
    }
    if (a.syms.size() != b.syms.size()) {
        return a.syms.size() < b.syms.size() ? -1 : 1;
    }
    const std::size_t m = std::min(a.funs.size(), b.funs.size());
    for (std::size_t i = 0; i < m; ++i) {
        const int c = cmp_tfun<O>(a.funs[i], b.funs[i]);
        if (c != 0) {
            return c;
        }
    }
    if (a.funs.size() != b.funs.size()) {
        return a.funs.size() < b.funs.size() ? -1 : 1;
    }
    if (!with_coef) {
        return 0;
    }
    const int c = cmp_vec(a.num, b.num);
    if (c != 0) {
        return c;
    }
    return cmp_vec(a.den, b.den);
}

void add_vec(std::vector<std::uint64_t>& s, const std::vector<std::uint64_t>& b)
{
    add_limbs(s, b.data(), b.size());
}

template <Order O>
std::vector<TPtr> sort_merge_tree(std::vector<TPtr> v)
{
    std::sort(v.begin(), v.end(), [](const TPtr& x, const TPtr& y) { return cmp_tree<O>(*x, *y, false) < 0; });
    std::vector<TPtr> out;
    out.reserve(v.size());
    for (std::size_t i = 0; i < v.size();) {
        std::size_t j = i + 1;
        while (j < v.size() && cmp_tree<O>(*v[i], *v[j], false) == 0) {
            ++j;
        }
        if (j == i + 1) {
            out.push_back(v[i]);
        } else {
            auto t = std::make_shared<TTerm>(*v[i]);
            for (std::size_t k = i + 1; k < j; ++k) {
                add_vec(t->num, v[k]->num);
            }
            out.push_back(std::move(t));
        }
        i = j;
    }
    return out;
}

// ---------------------------------------------------------------------------------------
// Printing and digests: every representation prints a term the same way, so equal
// digests mean equal expressions in the same order.

struct Fnv1a {
    std::uint64_t h = 0xcbf29ce484222325ull;
    void add(const std::string& s)
    {
        for (const char ch : s) {
            h ^= static_cast<unsigned char>(ch);
            h *= 0x100000001b3ull;
        }
    }
};

template <class W>
void append_hex(std::string& s, const W* limbs, std::size_t n)
{
    static const char* digits = "0123456789abcdef";
    bool leading = true;
    for (std::size_t i = n; i-- > 0;) {
        for (int shift = static_cast<int>(sizeof(W) * 8) - 4; shift >= 0; shift -= 4) {
            const auto v = static_cast<unsigned>((limbs[i] >> shift) & 0xFu);
            if (leading && v == 0) {
                continue;
            }
            leading = false;
            s += digits[v];
        }
    }
    if (leading) {
        s += '0';
    }
}

void append_sym(std::string& s, bool& first, std::uint64_t sym, std::uint64_t pow)
{
    if (!first) {
        s += '*';
    }
    first = false;
    s += 's';
    s += std::to_string(sym);
    if (pow != 1) {
        s += '^';
        s += std::to_string(pow);
    }
}

template <class W>
void render_flat(const W* t, std::string& s)
{
    const W* p = t + 1;
    bool first = true;
    for (;;) {
        if (p[0] == kSym) {
            const std::size_t n = (len(p[1]) - 2) / 2;
            for (std::size_t i = 0; i < n; ++i) {
                append_sym(s, first, p[2 + 2 * i], p[3 + 2 * i]);
            }
        } else if (p[0] == kFun) {
            if (!first) {
                s += '*';
            }
            first = false;
            s += 'f';
            s += std::to_string(static_cast<std::uint64_t>(p[2]));
            s += '(';
            const W* a = p + 3;
            const W* e = p + len(p[1]);
            bool first_arg = true;
            while (a < e) {
                if (!first_arg) {
                    s += ',';
                }
                first_arg = false;
                const W* x = a + 2;
                const W* xe = a + len(a[1]);
                bool first_term = true;
                while (x < xe) {
                    if (!first_term) {
                        s += '+';
                    }
                    first_term = false;
                    render_flat(x, s);
                    x += len(x[0]);
                }
                a = xe;
            }
            s += ')';
        } else {
            if (!first) {
                s += '*';
            }
            const std::size_t nn = len(p[2]);
            s += '#';
            append_hex(s, p + 3, nn);
            s += '/';
            append_hex(s, p + 3 + nn, len(p[1]) - 3 - nn);
            return;
        }
        p += len(p[1]);
    }
}

void render_tree(const TTerm& t, std::string& s)
{
    bool first = true;
    for (const auto& [sym, pow] : t.syms) {
        append_sym(s, first, sym, pow);
    }
    for (const TFun& f : t.funs) {
        if (!first) {
            s += '*';
        }
        first = false;
        s += 'f';
        s += std::to_string(static_cast<std::uint64_t>(f.id));
        s += '(';
        for (std::size_t i = 0; i < f.args.size(); ++i) {
            if (i != 0) {
                s += ',';
            }
            for (std::size_t j = 0; j < f.args[i].size(); ++j) {
                if (j != 0) {
                    s += '+';
                }
                render_tree(*f.args[i][j], s);
            }
        }
        s += ')';
    }
    if (!first) {
        s += '*';
    }
    s += '#';
    append_hex(s, t.num.data(), t.num.size());
    s += '/';
    append_hex(s, t.den.data(), t.den.size());
}

template <class W>
std::uint64_t digest_flat(const std::vector<W>& words)
{
    Fnv1a h;
    std::string s;
    for (std::size_t p = 0; p < words.size(); p += len(words[p])) {
        s.clear();
        render_flat(words.data() + p, s);
        s += '\n';
        h.add(s);
    }
    return h.h;
}

std::uint64_t digest_tree(const std::vector<TPtr>& v)
{
    Fnv1a h;
    std::string s;
    for (const TPtr& t : v) {
        s.clear();
        render_tree(*t, s);
        s += '\n';
        h.add(s);
    }
    return h.h;
}

// ---------------------------------------------------------------------------------------
// The engine side of the module interface (32-bit encoding, rank order).

struct TermBuilder {
    std::vector<std::uint32_t>* out = nullptr;
    std::vector<SymPow> pairs;
    const std::uint32_t* rest = nullptr;

    void begin()
    {
        pairs.clear();
        rest = nullptr;
    }
    void add_symbol(std::uint32_t s, std::uint32_t p) { pairs.emplace_back(s, p); }
    void copy_rest(const std::uint32_t* term) { rest = term; }
    void end()
    {
        for (std::size_t i = 1; i < pairs.size(); ++i) {
            const SymPow x = pairs[i];
            std::size_t j = i;
            while (j > 0 && g_rank[pairs[j - 1].first] > g_rank[x.first]) {
                pairs[j] = pairs[j - 1];
                --j;
            }
            pairs[j] = x;
        }
        std::vector<std::uint32_t>& w = *out;
        const std::size_t at = w.size();
        w.push_back(0);
        if (!pairs.empty()) {
            w.push_back(kSym);
            w.push_back(static_cast<std::uint32_t>(2 + 2 * pairs.size()));
            for (const auto& [s, p] : pairs) {
                w.push_back(s);
                w.push_back(p);
            }
        }
        const std::uint32_t* p = rest + 1;
        const std::uint32_t* e = rest + rest[0];
        if (p[0] == kSym) {
            p += p[1];
        }
        w.insert(w.end(), p, e);
        w[at] = static_cast<std::uint32_t>(w.size() - at);
    }
};

// The second wrapper of ADR-0004: the same module code, reading the encoding directly.
struct DirectView {
    const std::uint32_t* t;
    std::uint32_t symbol_count() const { return t[1] == kSym ? (t[2] - 2) / 2 : 0; }
    SymPow symbol_at(std::uint32_t i) const { return {t[3 + 2 * i], t[4 + 2 * i]}; }
};

struct DirectBuilder {
    TermBuilder& b;
    void begin() { b.begin(); }
    void add_symbol(std::uint32_t s, std::uint32_t p) { b.add_symbol(s, p); }
    void copy_rest(const DirectView& v) { b.copy_rest(v.t); }
    void end() { b.end(); }
};

const std::uint32_t* as_words(const tante_term* t) { return reinterpret_cast<const std::uint32_t*>(t); }
TermBuilder* as_builder(tante_builder* b) { return reinterpret_cast<TermBuilder*>(b); }

std::uint32_t host_symbol_count(const tante_term* t) { return DirectView{as_words(t)}.symbol_count(); }
void host_symbol_at(const tante_term* t, std::uint32_t i, std::uint32_t* s, std::uint32_t* p)
{
    const SymPow x = DirectView{as_words(t)}.symbol_at(i);
    *s = x.first;
    *p = x.second;
}
void host_begin(tante_builder* b) { as_builder(b)->begin(); }
void host_add_symbol(tante_builder* b, std::uint32_t s, std::uint32_t p) { as_builder(b)->add_symbol(s, p); }
void host_copy_rest(tante_builder* b, const tante_term* t) { as_builder(b)->copy_rest(as_words(t)); }
void host_end(tante_builder* b) { as_builder(b)->end(); }
std::uint32_t host_term_symbols(const tante_term* t, std::uint32_t* pairs, std::uint32_t capacity)
{
    const DirectView v{as_words(t)};
    const std::uint32_t n = v.symbol_count();
    const std::uint32_t m = n < capacity ? n : capacity;
    for (std::uint32_t i = 0; i < m; ++i) {
        const SymPow x = v.symbol_at(i);
        pairs[2 * i] = x.first;
        pairs[2 * i + 1] = x.second;
    }
    return n;
}
void host_builder_write(tante_builder* b, const std::uint32_t* pairs, std::uint32_t count, const tante_term* rest)
{
    TermBuilder& tb = *as_builder(b);
    tb.begin();
    for (std::uint32_t i = 0; i < count; ++i) {
        tb.add_symbol(pairs[2 * i], pairs[2 * i + 1]);
    }
    tb.copy_rest(as_words(rest));
    tb.end();
}

const tante_host g_host = {1,
                           &host_symbol_count,
                           &host_symbol_at,
                           &host_begin,
                           &host_add_symbol,
                           &host_copy_rest,
                           &host_end,
                           &host_term_symbols,
                           &host_builder_write};

// ---------------------------------------------------------------------------------------
// Running and reporting.

using Clock = std::chrono::steady_clock;

template <class F>
double median_seconds(int reps, F&& f)
{
    std::vector<double> t;
    for (int r = 0; r < reps; ++r) {
        const auto a = Clock::now();
        f();
        const auto b = Clock::now();
        t.push_back(std::chrono::duration<double>(b - a).count());
    }
    std::sort(t.begin(), t.end());
    return t[t.size() / 2];
}

struct Report {
    std::FILE* results = nullptr;
    std::FILE* timings = nullptr;
    std::FILE* memory = nullptr;
    bool ok = true;

    void check(bool cond, const char* what)
    {
        if (!cond) {
            ok = false;
            std::fprintf(results, "MISMATCH %s\n", what);
            std::fprintf(stderr, "MISMATCH %s\n", what);
        }
    }
};

// bytes / n with one decimal, in integers so that it prints identically everywhere.
std::string per_term(std::size_t bytes, std::size_t n)
{
    const std::size_t tenths = (bytes * 10 + n / 2) / n;
    return std::to_string(tenths / 10) + "." + std::to_string(tenths % 10);
}

double ns_per_term(double seconds, std::size_t n) { return seconds * 1e9 / static_cast<double>(n); }

struct Workload {
    const char* name;
    Kind kind;
    std::size_t n;
    bool with_tree;
    bool with_flat64;
};

void run_sort(Report& rep, const Workload& w, int reps)
{
    const std::uint64_t seed = 1000 + static_cast<std::uint64_t>(w.kind);
    std::fprintf(stderr, "sort %s n=%zu\n", w.name, w.n);
    std::size_t terms_out = 0;
    std::uint64_t d_rank = 0;

    {
        const FlatExpr<std::uint32_t> in = build_flat<std::uint32_t, Order::Id>(w.kind, w.n, seed);
        FlatExpr<std::uint32_t> out;
        const double s = median_seconds(reps, [&] { out = sort_merge<std::uint32_t, Order::Id>(in); });
        std::fprintf(rep.results, "%s n=%zu flat32 bytes_per_term=%s\n", w.name, w.n,
                     per_term(in.words.size() * 4, w.n).c_str());
        std::fprintf(rep.results, "%s n=%zu flat32-id terms_out=%zu digest=%016llx\n", w.name, w.n, out.offsets.size(),
                     static_cast<unsigned long long>(digest_flat(out.words)));
        std::fprintf(rep.timings, "sort %-8s n=%-9zu flat32-id    %8.1f ns/term\n", w.name, w.n, ns_per_term(s, w.n));
        std::fflush(rep.timings);
        terms_out = out.offsets.size();
    }
    {
        const FlatExpr<std::uint32_t> in = build_flat<std::uint32_t, Order::Rank>(w.kind, w.n, seed);
        FlatExpr<std::uint32_t> out;
        const double s = median_seconds(reps, [&] { out = sort_merge<std::uint32_t, Order::Rank>(in); });
        d_rank = digest_flat(out.words);
        std::fprintf(rep.results, "%s n=%zu flat32-rank terms_out=%zu digest=%016llx\n", w.name, w.n,
                     out.offsets.size(), static_cast<unsigned long long>(d_rank));
        std::fprintf(rep.timings, "sort %-8s n=%-9zu flat32-rank  %8.1f ns/term\n", w.name, w.n, ns_per_term(s, w.n));
        std::fflush(rep.timings);
        rep.check(out.offsets.size() == terms_out, "terms_out flat32-rank vs flat32-id");
    }
    if (w.with_flat64) {
        const FlatExpr<std::uint64_t> in = build_flat<std::uint64_t, Order::Rank>(w.kind, w.n, seed);
        FlatExpr<std::uint64_t> out;
        const double s = median_seconds(reps, [&] { out = sort_merge<std::uint64_t, Order::Rank>(in); });
        const std::uint64_t d = digest_flat(out.words);
        std::fprintf(rep.results, "%s n=%zu flat64 bytes_per_term=%s\n", w.name, w.n,
                     per_term(in.words.size() * 8, w.n).c_str());
        std::fprintf(rep.results, "%s n=%zu flat64-rank terms_out=%zu digest=%016llx\n", w.name, w.n,
                     out.offsets.size(), static_cast<unsigned long long>(d));
        std::fprintf(rep.timings, "sort %-8s n=%-9zu flat64-rank  %8.1f ns/term\n", w.name, w.n, ns_per_term(s, w.n));
        std::fflush(rep.timings);
        rep.check(d == d_rank, "digest flat64-rank vs flat32-rank");
    }
    if (w.with_tree) {
        std::vector<TPtr> in;
        {
            SplitMix64 r{seed};
            ATerm t;
            in.reserve(w.n);
            [[maybe_unused]] const std::size_t live0 = g_live_bytes;
            [[maybe_unused]] const std::size_t alloc0 = g_allocations;
            g_counting = true;
            for (std::size_t i = 0; i < w.n; ++i) {
                generate(w.kind, r, t);
                g_counting = true;
                in.push_back(to_tree<Order::Rank>(t));
                g_counting = false; // the generator's own allocations are not the tree's
            }
            g_counting = false;
#ifndef SPIKE_SANITIZE
            std::fprintf(rep.memory, "%-8s n=%-9zu tree heap bytes_per_term=%s allocations_per_term=%s"
                                     " (plus %zu bytes per term for the array of pointers)\n",
                         w.name, w.n, per_term(g_live_bytes - live0, w.n).c_str(),
                         per_term((g_allocations - alloc0) * 10, w.n * 10).c_str(), sizeof(TPtr));
#endif
        }
        std::vector<TPtr> out;
        const double s = median_seconds(reps, [&] {
            std::vector<TPtr> copy;
            {
                // The copy of the pointer array is the caller's, not the sort's; it is
                // inside the timing only because it must be redone on every repetition.
                copy = in;
            }
            out = sort_merge_tree<Order::Rank>(std::move(copy));
        });
        const std::uint64_t d = digest_tree(out);
        std::fprintf(rep.results, "%s n=%zu tree-rank terms_out=%zu digest=%016llx\n", w.name, w.n, out.size(),
                     static_cast<unsigned long long>(d));
        std::fprintf(rep.timings, "sort %-8s n=%-9zu tree-rank    %8.1f ns/term\n", w.name, w.n, ns_per_term(s, w.n));
        std::fflush(rep.timings);
        rep.check(d == d_rank, "digest tree-rank vs flat32-rank");
    }
}

void run_calls(Report& rep, const tante_module_functions& fns, const char* name, Kind kind, std::size_t n, int reps)
{
    std::fprintf(stderr, "calls %s n=%zu\n", name, n);
    const std::uint64_t seed = 1000 + static_cast<std::uint64_t>(kind);
    const FlatExpr<std::uint32_t> in = build_flat<std::uint32_t, Order::Rank>(kind, n, seed);
    const std::uint32_t* base = in.words.data();
    constexpr std::uint32_t x = 2;

    std::int64_t sum_direct = 0;
    const double read_direct = median_seconds(reps, [&] {
        std::int64_t s = 0;
        for (const std::size_t o : in.offsets) {
            s += spike::sum_of_powers(DirectView{base + o});
        }
        sum_direct = s;
    });
    std::int64_t sum_c = 0;
    const double read_c = median_seconds(reps, [&] {
        std::int64_t s = 0;
        for (const std::size_t o : in.offsets) {
            s += fns.sum_of_powers(&g_host, reinterpret_cast<const tante_term*>(base + o));
        }
        sum_c = s;
    });
    std::int64_t sum_bulk = 0;
    const double read_bulk = median_seconds(reps, [&] {
        std::int64_t s = 0;
        for (const std::size_t o : in.offsets) {
            s += fns.sum_of_powers_bulk(&g_host, reinterpret_cast<const tante_term*>(base + o));
        }
        sum_bulk = s;
    });

    std::vector<std::uint32_t> out_direct;
    const double write_direct = median_seconds(reps, [&] {
        out_direct.clear();
        out_direct.reserve(in.words.size() + 2 * n);
        TermBuilder tb;
        tb.out = &out_direct;
        DirectBuilder db{tb};
        for (const std::size_t o : in.offsets) {
            spike::multiply_by_symbol(DirectView{base + o}, db, x);
        }
    });
    std::vector<std::uint32_t> out_c;
    const double write_c = median_seconds(reps, [&] {
        out_c.clear();
        out_c.reserve(in.words.size() + 2 * n);
        TermBuilder tb;
        tb.out = &out_c;
        for (const std::size_t o : in.offsets) {
            fns.multiply_by_symbol(&g_host, reinterpret_cast<const tante_term*>(base + o),
                                   reinterpret_cast<tante_builder*>(&tb), x);
        }
    });
    std::vector<std::uint32_t> out_bulk;
    const double write_bulk = median_seconds(reps, [&] {
        out_bulk.clear();
        out_bulk.reserve(in.words.size() + 2 * n);
        TermBuilder tb;
        tb.out = &out_bulk;
        for (const std::size_t o : in.offsets) {
            fns.multiply_by_symbol_bulk(&g_host, reinterpret_cast<const tante_term*>(base + o),
                                        reinterpret_cast<tante_builder*>(&tb), x);
        }
    });

    const std::uint64_t d_direct = digest_flat(out_direct);
    const std::uint64_t d_c = digest_flat(out_c);
    std::fprintf(rep.results, "calls %s n=%zu sum_of_powers=%lld multiply_digest=%016llx\n", name, n,
                 static_cast<long long>(sum_direct), static_cast<unsigned long long>(d_direct));
    rep.check(sum_direct == sum_c, "sum_of_powers direct vs C interface");
    rep.check(d_direct == d_c, "multiply_by_symbol direct vs C interface");
    rep.check(sum_direct == sum_bulk, "sum_of_powers direct vs C interface, bulk");
    rep.check(d_direct == digest_flat(out_bulk), "multiply_by_symbol direct vs C interface, bulk");
    const double rd = ns_per_term(read_direct, n);
    const double rc = ns_per_term(read_c, n);
    const double rb = ns_per_term(read_bulk, n);
    const double wd = ns_per_term(write_direct, n);
    const double wc = ns_per_term(write_c, n);
    const double wb = ns_per_term(write_bulk, n);
    std::fprintf(rep.timings,
                 "call %-8s n=%-9zu read   direct %7.1f  C %7.1f  C-bulk %7.1f ns/term  ratios %.2f %.2f\n", name, n,
                 rd, rc, rb, rc / rd, rb / rd);
    std::fprintf(rep.timings,
                 "call %-8s n=%-9zu write  direct %7.1f  C %7.1f  C-bulk %7.1f ns/term  ratios %.2f %.2f\n", name, n,
                 wd, wc, wb, wc / wd, wb / wd);
    std::fflush(rep.timings);
}

std::FILE* open_file(const std::filesystem::path& p)
{
    std::FILE* f = std::fopen(p.string().c_str(), "wb"); // binary: the same line endings everywhere
    if (f == nullptr) {
        std::fprintf(stderr, "cannot write %s\n", p.string().c_str());
        std::exit(2);
    }
    return f;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::fprintf(stderr, "usage: spike <output-directory> [quick]\n");
        return 2;
    }
    const bool quick = argc > 2 && std::string(argv[2]) == "quick";
    const std::filesystem::path dir = argv[1];
    std::filesystem::create_directories(dir);
    init_ranks();

    Report rep;
    rep.results = open_file(dir / "results.txt");
    rep.timings = open_file(dir / "timings.txt");
    rep.memory = open_file(dir / "memory.txt");

    {
        std::FILE* info = open_file(dir / "info.txt");
#if defined(_MSC_VER)
        std::fprintf(info, "compiler MSVC %d\n", _MSC_FULL_VER);
#elif defined(__clang__)
        std::fprintf(info, "compiler Clang %s\n", __clang_version__);
#else
        std::fprintf(info, "compiler GCC %s\n", __VERSION__);
#endif
        std::fprintf(info, "sizeof(void*) %zu, sizeof(TTerm) %zu, sizeof(std::shared_ptr) %zu\n", sizeof(void*),
                     sizeof(TTerm), sizeof(TPtr));
        std::fprintf(info, "mode %s\n", quick ? "quick" : "full");
        std::fclose(info);
    }

    tante_module_functions fns{};
    if (tante_module_entry(&g_host, &fns) != 0) {
        std::fprintf(stderr, "module refused the interface\n");
        return 2;
    }

    const std::size_t m = quick ? 20'000 : 1'000'000;
    const std::size_t big = quick ? 100'000 : 10'000'000;
    const int reps = quick ? 1 : 5;
    const Workload workloads[] = {
        {"poly4", Kind::Poly4, m, true, true},
        {"poly20", Kind::Poly20, m, true, true},
        {"bigcoef", Kind::BigCoef, m, true, true},
        {"nested", Kind::Nested, m, true, true},
        {"poly4", Kind::Poly4, big, false, true},
    };
    for (const Workload& w : workloads) {
        run_sort(rep, w, w.n == big ? (quick ? 1 : 3) : reps);
    }
    run_calls(rep, fns, "poly4", Kind::Poly4, m, reps);
    run_calls(rep, fns, "poly20", Kind::Poly20, m, reps);

    std::fclose(rep.results);
    std::fclose(rep.timings);
    std::fclose(rep.memory);
    return rep.ok ? 0 : 1;
}
