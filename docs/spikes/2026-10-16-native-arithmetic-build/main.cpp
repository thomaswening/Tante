// Spike for ADR-0001 §6: do GMP, MPFR and FLINT, obtained through vcpkg, build
// and give identical results with MSVC, GCC and Apple Clang?
// See https://github.com/thomaswening/Tante/blob/main/docs/adr/0001-cpp20-cmake-vcpkg-guardrails.md
//
// Writes three files into the directory given as the first argument:
//   results.txt  must be byte-identical on all systems (REQ-050, REQ-074)
//   timings.txt  median seconds per benchmark, expected to differ
//   info.txt     compiler and platform facts, expected to differ
// Exits with 1 if any check fails.

#include <gmp.h>
#include <mpfr.h>
#include <flint/flint.h>
#include <flint/fmpz.h>
#include <flint/fmpz_mpoly.h>
#include <flint/fmpz_mpoly_factor.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr std::array<const char*, 4> kVars{"x", "y", "z", "t"};

// A stable digest for large outputs, so results.txt stays small.
std::uint64_t fnv1a(const std::string& text)
{
    std::uint64_t hash = 14695981039346656037ULL;
    for (const char c : text) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

std::string fmpz_string(const fmpz_t value)
{
    char* raw = fmpz_get_str(nullptr, 10, value);
    std::string text(raw);
    flint_free(raw);
    return text;
}

// RAII owners for FLINT's C objects: the constructor initialises, the
// destructor clears, and copying is forbidden (like a sealed IDisposable).
class Context {
public:
    explicit Context(slong nvars) { fmpz_mpoly_ctx_init(ctx_, nvars, ORD_LEX); }
    ~Context() { fmpz_mpoly_ctx_clear(ctx_); }
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    fmpz_mpoly_ctx_struct* get() { return ctx_; }

private:
    fmpz_mpoly_ctx_t ctx_;
};

class Poly {
public:
    explicit Poly(Context& ctx) : ctx_(ctx) { fmpz_mpoly_init(p_, ctx_.get()); }
    Poly(Context& ctx, const std::string& text) : Poly(ctx)
    {
        if (fmpz_mpoly_set_str_pretty(p_, text.c_str(), vars(), ctx_.get()) != 0) {
            throw std::runtime_error("cannot parse: " + text);
        }
    }
    ~Poly() { fmpz_mpoly_clear(p_, ctx_.get()); }
    Poly(const Poly&) = delete;
    Poly& operator=(const Poly&) = delete;

    fmpz_mpoly_struct* get() { return p_; }
    const fmpz_mpoly_struct* get() const { return p_; }

    std::string str() const
    {
        char* raw = fmpz_mpoly_get_str_pretty(p_, vars(), ctx_.get());
        std::string text(raw);
        flint_free(raw);
        return text;
    }

    long long length() const { return static_cast<long long>(fmpz_mpoly_length(p_, ctx_.get())); }

    std::string digest() const
    {
        const std::string text = str();
        return std::format("{} terms, {} characters, fnv1a {:016x}", length(), text.size(), fnv1a(text));
    }

private:
    static const char** vars() { return const_cast<const char**>(kVars.data()); }

    Context& ctx_;
    fmpz_mpoly_t p_;
};

class FactorResult {
public:
    explicit FactorResult(Context& ctx) : ctx_(ctx) { fmpz_mpoly_factor_init(f_, ctx_.get()); }
    ~FactorResult() { fmpz_mpoly_factor_clear(f_, ctx_.get()); }
    FactorResult(const FactorResult&) = delete;
    FactorResult& operator=(const FactorResult&) = delete;

    fmpz_mpoly_factor_struct* get() { return f_; }

private:
    Context& ctx_;
    fmpz_mpoly_factor_t f_;
};

template <typename F>
double median_seconds(int repetitions, F&& work)
{
    std::vector<double> seconds;
    for (int i = 0; i < repetitions; ++i) {
        const auto start = std::chrono::steady_clock::now();
        work();
        seconds.push_back(std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count());
    }
    std::sort(seconds.begin(), seconds.end());
    return seconds[seconds.size() / 2];
}

std::string compiler()
{
#if defined(_MSC_VER) && !defined(__clang__)
    return std::format("MSVC {}", _MSC_FULL_VER);
#elif defined(__clang__)
    return std::format("Clang {}", __clang_version__);
#elif defined(__GNUC__)
    return std::format("GCC {}", __VERSION__);
#else
    return "unknown";
#endif
}

} // namespace

int main(int argc, char** argv)
{
    try {
        const std::filesystem::path out = argc > 1 ? argv[1] : "out";
        std::filesystem::create_directories(out);
        std::ofstream results(out / "results.txt", std::ios::binary);
        std::ofstream timings(out / "timings.txt", std::ios::binary);
        std::ofstream info(out / "info.txt", std::ios::binary);

        bool ok = true;
        auto check = [&](bool passed, const std::string& what) {
            results << "check " << what << ": " << (passed ? "pass" : "FAIL") << '\n';
            ok = ok && passed;
        };

        // Question 2: widths of the arithmetic types.
        info << "compiler: " << compiler() << '\n';
        info << "sizeof(long): " << sizeof(long) << '\n';
        info << "sizeof(void*): " << sizeof(void*) << '\n';
        results << "GMP " << gmp_version << ", MPFR " << mpfr_get_version() << ", FLINT " << FLINT_VERSION << '\n';
        results << "GMP_LIMB_BITS " << GMP_LIMB_BITS << ", sizeof(mp_limb_t) " << sizeof(mp_limb_t)
                << ", FLINT_BITS " << FLINT_BITS << ", sizeof(ulong) " << sizeof(ulong)
                << ", sizeof(slong) " << sizeof(slong) << '\n';
        check(GMP_LIMB_BITS == 64 && FLINT_BITS == 64 && sizeof(ulong) == 8, "64-bit limbs");

        Context ctx(static_cast<slong>(kVars.size()));

        // The canonical-form example of REQ-012.
        {
            const Poly p(ctx, "(x+y)^2 - x^2");
            results << "(x+y)^2 - x^2 = " << p.str() << '\n';
        }

        // A small GCD.
        {
            Poly a(ctx, "x^2 - y^2");
            Poly b(ctx, "x^2 + 2*x*y + y^2");
            Poly g(ctx);
            check(fmpz_mpoly_gcd(g.get(), a.get(), b.get(), ctx.get()) != 0, "small gcd computed");
            results << "gcd(x^2 - y^2, x^2 + 2*x*y + y^2) = " << g.str() << '\n';
        }

        // A factorisation over the rationals (REQ-010), checked by multiplying back.
        {
            Poly a(ctx, "(x^2 - y^2)*(1 + x + y + z)^3*(x*y*z + 1)*(x + 2*y + 3*z - 5)^2*6");
            FactorResult f(ctx);
            check(fmpz_mpoly_factor(f.get(), a.get(), ctx.get()) != 0, "factorisation computed");
            results << "factorisation: constant " << fmpz_string(f.get()->constant) << '\n';
            std::vector<std::string> factors;
            for (slong i = 0; i < f.get()->num; ++i) {
                Poly factor(ctx);
                fmpz_mpoly_set(factor.get(), f.get()->poly + i, ctx.get());
                factors.push_back(std::format("({})^{}", factor.str(), fmpz_string(f.get()->exp + i)));
            }
            for (const auto& factor : factors) {
                results << "  as returned: " << factor << '\n';
            }
            std::sort(factors.begin(), factors.end());
            for (const auto& factor : factors) {
                results << "  sorted:      " << factor << '\n';
            }
            Poly back(ctx);
            check(fmpz_mpoly_factor_expand(back.get(), f.get(), ctx.get()) != 0
                      && fmpz_mpoly_equal(back.get(), a.get(), ctx.get()) != 0,
                  "factors multiply back");
        }

        // Numerical evaluation through MPFR (REQ-101).
        {
            mpfr_t e;
            mpfr_init2(e, 128);
            mpfr_set_ui(e, 1, MPFR_RNDN);
            mpfr_exp(e, e, MPFR_RNDN);
            mpfr_exp_t exponent = 0;
            char* digits = mpfr_get_str(nullptr, &exponent, 10, 30, e, MPFR_RNDN);
            results << "exp(1) to 30 digits: " << digits << " e" << exponent << '\n';
            mpfr_free_str(digits);
            mpfr_clear(e);
        }

        // Question 5: benchmarks. Results go to results.txt, times to timings.txt.
        info << "FLINT threads: " << flint_get_num_threads() << '\n';
        {
            mpz_t a;
            mpz_t b;
            mpz_t c;
            mpz_inits(a, b, c, nullptr);
            mpz_ui_pow_ui(a, 3, 6000000);
            mpz_ui_pow_ui(b, 7, 5000000);
            const double s = median_seconds(5, [&] { mpz_mul(c, a, b); });
            timings << std::format("gmp_mul_3^6e6_x_7^5e6 {:.4f}\n", s);
            results << "3^6000000 * 7^5000000: " << mpz_sizeinbase(c, 2) << " bits, mod 1000000007 = "
                    << mpz_fdiv_ui(c, 1000000007UL) << '\n';
            mpz_clears(a, b, c, nullptr);
        }
        {
            Poly f(ctx, "(1 + x + y + z + t)^20");
            Poly g(ctx);
            fmpz_mpoly_add_ui(g.get(), f.get(), 1, ctx.get());
            Poly p(ctx);
            const double s = median_seconds(3, [&] { fmpz_mpoly_mul(p.get(), f.get(), g.get(), ctx.get()); });
            timings << std::format("poly_mul_f20_x_f20+1 {:.4f}\n", s);
            results << "f20*(f20+1): " << p.digest() << '\n';
        }
        {
            Poly f(ctx, "(1 + x + y + z + t)^15");
            Poly f1(ctx);
            Poly f2(ctx);
            fmpz_mpoly_add_ui(f1.get(), f.get(), 1, ctx.get());
            fmpz_mpoly_add_ui(f2.get(), f.get(), 2, ctx.get());
            Poly a(ctx);
            Poly b(ctx);
            fmpz_mpoly_mul(a.get(), f.get(), f1.get(), ctx.get());
            fmpz_mpoly_mul(b.get(), f.get(), f2.get(), ctx.get());
            Poly g(ctx);
            const double s = median_seconds(3, [&] { fmpz_mpoly_gcd(g.get(), a.get(), b.get(), ctx.get()); });
            timings << std::format("poly_gcd_f15(f15+1)_f15(f15+2) {:.4f}\n", s);
            results << "gcd(f15*(f15+1), f15*(f15+2)): " << g.digest() << '\n';
            check(fmpz_mpoly_equal(g.get(), f.get(), ctx.get()) != 0, "large gcd equals f15");
        }
        {
            Poly a(ctx, "((1 + x + y + z + t)^5 + x*y)*((1 + x + y + z + t)^4 + z*t + 2)");
            Poly back(ctx);
            const double s = median_seconds(3, [&] {
                FactorResult f(ctx);
                fmpz_mpoly_factor(f.get(), a.get(), ctx.get());
                fmpz_mpoly_factor_expand(back.get(), f.get(), ctx.get());
            });
            timings << std::format("poly_factor_two_dense_factors {:.4f}\n", s);
            check(fmpz_mpoly_equal(back.get(), a.get(), ctx.get()) != 0, "dense factors multiply back");
        }

        results << (ok ? "ALL CHECKS PASSED" : "SOME CHECKS FAILED") << '\n';
        std::cout << (ok ? "ALL CHECKS PASSED" : "SOME CHECKS FAILED") << '\n';
        return ok ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
