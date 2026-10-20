// The spike's module: a shared library that reaches the engine only through the
// table of C functions it is given, wrapped in a small C++ header-style wrapper.
#include "spike_module.h"

#include "module_logic.hpp"

#include <utility>

namespace {

class TermView {
public:
    TermView(const tante_host* host, const tante_term* term) : host_(host), term_(term) {}
    std::uint32_t symbol_count() const { return host_->term_symbol_count(term_); }
    std::pair<std::uint32_t, std::uint32_t> symbol_at(std::uint32_t index) const
    {
        std::uint32_t s = 0;
        std::uint32_t p = 0;
        host_->term_symbol_at(term_, index, &s, &p);
        return {s, p};
    }
    const tante_term* raw() const { return term_; }

private:
    const tante_host* host_;
    const tante_term* term_;
};

class Builder {
public:
    Builder(const tante_host* host, tante_builder* builder) : host_(host), builder_(builder) {}
    void begin() { host_->builder_begin(builder_); }
    void add_symbol(std::uint32_t s, std::uint32_t p) { host_->builder_add_symbol(builder_, s, p); }
    void copy_rest(const TermView& term) { host_->builder_copy_rest(builder_, term.raw()); }
    void end() { host_->builder_end(builder_); }

private:
    const tante_host* host_;
    tante_builder* builder_;
};

std::int64_t sum_of_powers_c(const tante_host* host, const tante_term* term)
{
    return spike::sum_of_powers(TermView(host, term));
}

void multiply_by_symbol_c(const tante_host* host, const tante_term* term, tante_builder* out, std::uint32_t symbol)
{
    Builder builder(host, out);
    spike::multiply_by_symbol(TermView(host, term), builder, symbol);
}

} // namespace

extern "C" SPIKE_MODULE_API int tante_module_entry(const tante_host* host, tante_module_functions* out)
{
    if (host == nullptr || out == nullptr || host->interface_version != 1) {
        return 1;
    }
    out->sum_of_powers = &sum_of_powers_c;
    out->multiply_by_symbol = &multiply_by_symbol_c;
    return 0;
}
