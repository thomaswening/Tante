# Spike: GMP, MPFR and FLINT build natively through vcpkg with MSVC, GCC and Apple Clang, and give identical results

Date: 16 October 2026 | For: [ADR-0001](../adr/0001-cpp20-cmake-vcpkg-guardrails.md) §6, [ADR-0003](../adr/0003-flint-gmp-mpfr-behind-own-interfaces.md), [REQ-050](../requirements.md#req-050), [REQ-074](../requirements.md#req-074)

## Question

From [ADR-0001](../adr/0001-cpp20-cmake-vcpkg-guardrails.md) §6, on the GitHub-hosted runners of all three systems:

1. Does a CMake project with a vcpkg manifest build GMP, MPFR and FLINT and link a C++20 program that multiplies multivariate polynomials, takes their GCD and factorises one, with MSVC, GCC and Apple Clang?
2. Are GMP's limbs and FLINT's `ulong` 64 bits wide on all three?
3. Is the printed result byte-identical on all three?
4. How long does the vcpkg build take, cold and with binary caching?
5. How much slower are the MSVC builds of GMP and FLINT than the Linux builds on a short polynomial benchmark, and is the difference large enough to prefer clang-cl or MinGW-w64 on Windows?
6. Do AddressSanitizer and UndefinedBehaviorSanitizer run cleanly over the program on each system?

## Method

The code lives on the branch `spike/native-arithmetic-build`, which is kept and not merged, so that its CI workflow does not run on `main`. Final state: commit [`829f634`](https://github.com/thomaswening/Tante/tree/829f634d3ae1d0960198b5ab7d6507d1693ce59b/docs/spikes/2026-10-16-native-arithmetic-build):

- [`main.cpp`](https://github.com/thomaswening/Tante/blob/829f634d3ae1d0960198b5ab7d6507d1693ce59b/docs/spikes/2026-10-16-native-arithmetic-build/main.cpp): a C++20 program that wraps FLINT's `fmpz_mpoly` objects in RAII classes and writes three files. `results.txt` must be identical everywhere: library versions and type widths; the canonical-form example of [REQ-012](../requirements.md#req-012), (x+y)² − x² = 2xy + y²; a small GCD; a factorisation over the rationals of 6·(x²−y²)(1+x+y+z)³(xyz+1)(x+2y+3z−5)², checked by multiplying back ([REQ-010](../requirements.md#req-010)); exp(1) to 30 digits with MPFR at 128 bits ([REQ-101](../requirements.md#req-101)); and the results of four benchmarks, large outputs as a term count, length and FNV-1a digest of the printed polynomial. `timings.txt` holds the median time of each benchmark; `info.txt` compiler and platform facts. The program exits non-zero if any check fails.
- Benchmarks (single-threaded; FLINT reported 1 thread): GMP multiplication 3^6000000 · 7^5000000 (median of 5); the product f·(f+1) with f = (1+x+y+z+t)²⁰, 135,751 terms (median of 3); gcd(f·(f+1), f·(f+2)) with f = (1+x+y+z+t)¹⁵, 3,876 terms (median of 3); factorisation of ((1+x+y+z+t)⁵+xy)·((1+x+y+z+t)⁴+zt+2) (median of 3). These are polybench-style constructions written for this spike; no polybench data, and nothing from Symbolica ([ADR-0002](../adr/0002-no-input-from-symbolica.md)), was used.
- [`CMakeLists.txt`](https://github.com/thomaswening/Tante/blob/829f634d3ae1d0960198b5ab7d6507d1693ce59b/docs/spikes/2026-10-16-native-arithmetic-build/CMakeLists.txt): C++20 without extensions, the warning flags of [ADR-0001](../adr/0001-cpp20-cmake-vcpkg-guardrails.md) §3 as errors (`/W4 /WX /permissive- /utf-8`; `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Werror`), library headers as system headers, and an option for sanitizers (`-fsanitize=address,undefined -fno-sanitize-recover=all` with GCC and Clang, `/fsanitize=address` with MSVC).
- [`vcpkg.json`](https://github.com/thomaswening/Tante/blob/829f634d3ae1d0960198b5ab7d6507d1693ce59b/docs/spikes/2026-10-16-native-arithmetic-build/vcpkg.json): `gmp`, `mpfr`, `flint`, baseline vcpkg commit `c52f7bd7` (15 October 2026).
- [Workflow](https://github.com/thomaswening/Tante/blob/829f634d3ae1d0960198b5ab7d6507d1693ce59b/.github/workflows/spike-native-arithmetic-build.yml): on `windows-latest` (image windows-2025-vs2026, triplet `x64-windows`, dynamic libraries), `ubuntu-latest` (ubuntu-24.04, `x64-linux`, static) and `macos-latest` (macos-26-arm64, `arm64-osx`, static): clone vcpkg at the pinned commit, configure (vcpkg builds the libraries here, timed), build, run, build and run again with sanitizers and require the same `results.txt`, then a final job requires `results.txt` byte-identical across the three. vcpkg's binary cache is kept with `actions/cache`, keyed on triplet, vcpkg commit and manifest.

Runs: [38826077256](https://github.com/thomaswening/Tante/actions/runs/38826077256) (cold cache, all green) and [38831077434](https://github.com/thomaswening/Tante/actions/runs/38831077434) (warm cache, all green, CPU recorded). An earlier run, [38825259189](https://github.com/thomaswening/Tante/actions/runs/38825259189), passed on Windows and failed on Linux and macOS, as described below.

Two fixes were needed before the runs went green, neither about the libraries: the workflow used the `runner` context where GitHub does not allow it; and GCC and Apple Clang rejected an implicit `char` to `unsigned char` conversion under `-Wsign-conversion` (MSVC's `/W4` accepted it). The second is the warning guardrail doing its job.

## Result

| # | Question | Answer | Observed |
|---|---|---|---|
| 1 | Builds and links on all three? | **Yes** | GMP 6.3.0, MPFR 4.2.2, FLINT 3.6.0 built by vcpkg with MSVC 19.51.36260 (Visual Studio 2026), GCC 13.3.0 and Apple Clang 21.0.0 (Xcode 26.6); all checks passed on all three in all three runs. |
| 2 | 64-bit limbs and `ulong`? | **Yes** | `GMP_LIMB_BITS` 64, `FLINT_BITS` 64, `sizeof(ulong)` = `sizeof(slong)` = 8 everywhere, although `sizeof(long)` is 4 on Windows and 8 elsewhere: FLINT's `ulong` is not C's `unsigned long`. |
| 3 | Identical output? | **Yes** | `results.txt` byte-identical on the three systems, in both green runs, and identical between runs; this includes the order in which FLINT returned the five factors. |
| 4 | Build time, cold / warm | Linux 486 s / 6 s; macOS 372 s / 8 s; Windows 2,954 s / 23 s (configure step) | Cold, per library: Windows GMP 19 min, MPFR 11 min, FLINT 18 min; Linux 1.8 min, 57 s, 6.5 min; macOS 1.8, 1.5, 2.7 min. Whole job cold / warm: Linux 8 min 52 s / 1 min 10 s; macOS 6 min 55 s / 52 s; Windows 51 min 10 s / 1 min 56 s. Cache sizes: 55 MB, 18 MB, 38 MB. |
| 5 | MSVC slower than Linux? | **Slightly, not enough to change toolchain** | Warm run, both jobs on an AMD EPYC 9V74 with 2 vCPUs (seconds, Windows / Linux, ratio): GMP multiply 0.0582 / 0.0560 (1.04); poly multiply 0.2685 / 0.2145 (1.25); GCD 0.7353 / 0.6971 (1.05); factorisation 0.0024 / 0.0018 (1.33). macOS (Apple M2 Pro, virtual): 0.0329, 0.0886, 0.4159, 0.0012. In the cold run, the Linux job (CPU not recorded) was about twice as fast as in the warm run on every benchmark, so the runner's hardware varies more than the compiler does. |
| 6 | Sanitizers clean? | **Yes** | ASan + UBSan (no recovery) on Linux and macOS, ASan on Windows: no reports, same `results.txt`. |

GMP's assembly on Windows: vcpkg's GMP port at the pinned commit ([portfile](https://github.com/microsoft/vcpkg/blob/c52f7bd748185ef0e0daf033d9f2ddfc8a3449ea/ports/gmp/portfile.cmake)) keeps GMP's hand-written assembly with MSVC on x64, assembling it with Clang (`--target=x86_64-pc-windows-msvc`); it disables assembly only on Windows ARM64. The MSVC GMP multiplication, within 4 % of Linux on the same CPU, is consistent with that.

## Not tested

- **The sanitizers covered the spike's own code, not the libraries**, which vcpkg built without instrumentation. A memory error inside FLINT would only be seen where it corrupts Tante's data.
- **Compiler minimums.** Only the runners' current compilers were used: MSVC from Visual Studio 2026, not 2022 17.10; Apple Clang 21, not 16; GCC 13.3. [ADR-0001](../adr/0001-cpp20-cmake-vcpkg-guardrails.md)'s minimums remain unconfirmed.
- **CPU-specific builds and the binary cache.** GMP's configure tunes for the CPU of the machine that builds it, and vcpkg's `fat` feature (runtime selection of routines) is not supported on Windows with MSVC. A cached GMP built on one runner CPU and restored on an older one could use instructions the latter lacks. It did not happen here; the release builds and the cache key must deal with it.
- **FLINT's own assembly.** On Windows vcpkg builds FLINT with its CMake build, elsewhere with its autotools build; whether they enable the same optimised code was not checked. The 25 % gap on polynomial multiplication may come from there, or from noise.
- **Timing noise.** One run each, medians of 3 to 5 repetitions, on shared 2-vCPU virtual machines; differences under about 30 % should not be read as real.
- **Linux on ARM64, macOS on x86-64, clang-cl and MinGW-w64** were not built. Windows on ARM64 is excluded by the FLINT port.
- **Python wheels**, dynamic loading of compiled modules and the C interface ([REQ-032](../requirements.md#req-032)) were outside this spike.

## Conclusion

The route [ADR-0001](../adr/0001-cpp20-cmake-vcpkg-guardrails.md) proposes works: vcpkg builds GMP, MPFR and FLINT natively with MSVC, GCC and Apple Clang, the three give byte-identical results on polynomial multiplication, GCD, factorisation and MPFR evaluation, the sanitizers are clean, and MSVC's builds are within about 5 % of GCC's on GMP multiplication and GCD on the same CPU and about 25 % slower on polynomial multiplication. There is no reason to fall back to clang-cl or MinGW-w64. The cost is a cold Windows build of about 50 minutes, so vcpkg's binary cache is needed in CI from the first commit; with it, a run takes one to two minutes on each system. Adopted in [ADR-0001](../adr/0001-cpp20-cmake-vcpkg-guardrails.md) §6 and [ADR-0003](../adr/0003-flint-gmp-mpfr-behind-own-interfaces.md).
