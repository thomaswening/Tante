// The module's own code, written once against a wrapper API. It is compiled twice:
// in the shared library against the wrapper over the C interface, and in the engine
// against a wrapper that reads the encoding directly. This is the remedy ADR-0004
// names (a second wrapper implementation), so the spike measures exactly that choice.
#pragma once

#include <cstdint>

namespace spike {

// Read-only: the sum of the powers of all symbols in a term.
template <class View>
std::int64_t sum_of_powers(const View& term)
{
    std::int64_t sum = 0;
    const std::uint32_t n = term.symbol_count();
    for (std::uint32_t i = 0; i < n; ++i) {
        sum += term.symbol_at(i).second;
    }
    return sum;
}

// Rewriting: the term multiplied by one symbol.
template <class View, class Builder>
void multiply_by_symbol(const View& term, Builder& out, std::uint32_t symbol)
{
    out.begin();
    bool found = false;
    const std::uint32_t n = term.symbol_count();
    for (std::uint32_t i = 0; i < n; ++i) {
        auto [s, p] = term.symbol_at(i);
        if (s == symbol) {
            ++p;
            found = true;
        }
        out.add_symbol(s, p);
    }
    if (!found) {
        out.add_symbol(symbol, 1);
    }
    out.copy_rest(term);
    out.end();
}

} // namespace spike
