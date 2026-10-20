// The spike's module: a shared library that reaches the engine only through the
// table of C functions it is given, wrapped in a small C++ header-style wrapper.
#include "spike_module.h"

#include "module_logic.hpp"

#include <array>
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

// The same wrapper API, implemented with one C call per term: the view copies the
// symbols out once, the builder collects them and hands them over in end(). A term with
// more symbols than the buffer holds falls back to one call per symbol.
constexpr std::uint32_t kBulkPairs = 64;

class BulkTermView {
public:
    BulkTermView(const tante_host* host, const tante_term* term)
        : host_(host), term_(term), count_(host->term_symbols(term, pairs_.data(), kBulkPairs))
    {
    }
    std::uint32_t symbol_count() const { return count_; }
    std::pair<std::uint32_t, std::uint32_t> symbol_at(std::uint32_t index) const
    {
        if (count_ <= kBulkPairs) {
            return {pairs_[2 * index], pairs_[2 * index + 1]};
        }
        std::uint32_t s = 0;
        std::uint32_t p = 0;
        host_->term_symbol_at(term_, index, &s, &p);
        return {s, p};
    }
    const tante_term* raw() const { return term_; }

private:
    const tante_host* host_;
    const tante_term* term_;
    std::array<std::uint32_t, 2 * kBulkPairs> pairs_; // not zero-filled: only the first count_ pairs are read
    std::uint32_t count_;
};

class BulkBuilder {
public:
    BulkBuilder(const tante_host* host, tante_builder* builder) : host_(host), builder_(builder) {}
    void begin()
    {
        count_ = 0;
        overflow_ = false;
    }
    void add_symbol(std::uint32_t s, std::uint32_t p)
    {
        if (!overflow_ && count_ == kBulkPairs) {
            overflow_ = true;
            host_->builder_begin(builder_);
            for (std::uint32_t i = 0; i < count_; ++i) {
                host_->builder_add_symbol(builder_, pairs_[2 * i], pairs_[2 * i + 1]);
            }
        }
        if (overflow_) {
            host_->builder_add_symbol(builder_, s, p);
            return;
        }
        pairs_[2 * count_] = s;
        pairs_[2 * count_ + 1] = p;
        ++count_;
    }
    void copy_rest(const BulkTermView& term) { rest_ = term.raw(); }
    void end()
    {
        if (overflow_) {
            host_->builder_copy_rest(builder_, rest_);
            host_->builder_end(builder_);
        } else {
            host_->builder_write(builder_, pairs_.data(), count_, rest_);
        }
    }

private:
    const tante_host* host_;
    tante_builder* builder_;
    const tante_term* rest_ = nullptr;
    std::array<std::uint32_t, 2 * kBulkPairs> pairs_; // not zero-filled: only the first count_ pairs are read
    std::uint32_t count_ = 0;
    bool overflow_ = false;
};

std::int64_t sum_of_powers_bulk_c(const tante_host* host, const tante_term* term)
{
    return spike::sum_of_powers(BulkTermView(host, term));
}

void multiply_by_symbol_bulk_c(const tante_host* host, const tante_term* term, tante_builder* out, std::uint32_t symbol)
{
    BulkBuilder builder(host, out);
    spike::multiply_by_symbol(BulkTermView(host, term), builder, symbol);
}

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
    out->sum_of_powers_bulk = &sum_of_powers_bulk_c;
    out->multiply_by_symbol_bulk = &multiply_by_symbol_bulk_c;
    return 0;
}
