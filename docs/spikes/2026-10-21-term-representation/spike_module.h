/* Spike for ADR-0005: a cut-down C module interface in the shape ADR-0004 decided.
 * A term is an opaque handle; the module reads it and writes a new term only through
 * functions in the table the engine passes to tante_module_entry. */
#ifndef TANTE_SPIKE_MODULE_H
#define TANTE_SPIKE_MODULE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#  ifdef SPIKE_MODULE_BUILD
#    define SPIKE_MODULE_API __declspec(dllexport)
#  else
#    define SPIKE_MODULE_API __declspec(dllimport)
#  endif
#else
#  define SPIKE_MODULE_API __attribute__((visibility("default")))
#endif

typedef struct tante_term tante_term;
typedef struct tante_builder tante_builder;

typedef struct tante_host {
    uint32_t interface_version;
    uint32_t (*term_symbol_count)(const tante_term* term);
    void (*term_symbol_at)(const tante_term* term, uint32_t index, uint32_t* symbol, uint32_t* power);
    void (*builder_begin)(tante_builder* builder);
    void (*builder_add_symbol)(tante_builder* builder, uint32_t symbol, uint32_t power);
    void (*builder_copy_rest)(tante_builder* builder, const tante_term* term);
    void (*builder_end)(tante_builder* builder);
    /* Bulk variants: one call per term instead of one per symbol. term_symbols copies up to
     * capacity (symbol, power) pairs and returns the number the term has. */
    uint32_t (*term_symbols)(const tante_term* term, uint32_t* pairs, uint32_t capacity);
    void (*builder_write)(tante_builder* builder, const uint32_t* pairs, uint32_t count, const tante_term* rest);
} tante_host;

typedef struct tante_module_functions {
    int64_t (*sum_of_powers)(const tante_host* host, const tante_term* term);
    void (*multiply_by_symbol)(const tante_host* host, const tante_term* term, tante_builder* out, uint32_t symbol);
    int64_t (*sum_of_powers_bulk)(const tante_host* host, const tante_term* term);
    void (*multiply_by_symbol_bulk)(const tante_host* host, const tante_term* term, tante_builder* out, uint32_t symbol);
} tante_module_functions;

/* Returns 0 on success. */
SPIKE_MODULE_API int tante_module_entry(const tante_host* host, tante_module_functions* out);

#ifdef __cplusplus
}
#endif

#endif
