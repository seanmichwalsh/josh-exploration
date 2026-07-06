#include <stdint.h>

/*
 * Header providing definitions for 'toy-core' unit's SW visible CSRs.
 * Auto generated from `//hardware/rtl/toy-core/core.sv`, do not hand modify.
 */

uint64_t set_field(uint64_t csr, uint64_t field_value, uint64_t field_size,
                   uint64_t field_offset) {
    uint64_t mask = ((1ULL << field_size) - 1) << field_offset;
    return (csr & ~mask) | (field_value << field_offset);
}

uint64_t get_field(uint64_t csr, uint64_t field_size, uint64_t field_offset) {
    return (csr>> field_offset) & ((1ULL << field_size) - 1);
}

/* =================================================================
 *  Name: Alpha
 *  Size: 8B
 *  Address: 0x800000
 *  Fields:
 *      field_a: [b00 - b15]
 *      field_b: [b16 - b23]
 *      field_c: [b24 - b55]
 *      UNUSED:  [b56 - b63]
 *
 * |----------------------------------------------------------------|
 * | UNUSED |           field_c            | field_b |   field_a    |
 * |----------------------------------------------------------------|
 * |63    56|55                          24|23     16|15           0|
 *
 * ==================================================================
 */
#define CSR_TOY_CORE_ALPHA uint64_t
#define CSR_TOY_CORE_ALPHA_ADDRESS 0x800000
#define CSR_TOY_CORE_ALPHA_size 0x8

CSR_TOY_CORE_ALPHA csr_toy_core_alpha_set_field_a(CSR_TOY_CORE_ALPHA alpha, uint64_t value) {
    uint64_t field_size = 16;
    uint64_t field_offset = 0;
    return set_field(alpha, value, field_size, field_offset);
}

uint64_t csr_toy_core_alpha_get_field_a(CSR_TOY_CORE_ALPHA alpha) {
    uint64_t field_size = 16;
    uint64_t field_offset = 0;
    return get_field(alpha, field_size, field_offset);
}

CSR_TOY_CORE_ALPHA csr_toy_core_alpha_set_field_b(CSR_TOY_CORE_ALPHA alpha, uint64_t value) {
    uint64_t field_size = 8;
    uint64_t field_offset = 16;
    return set_field(alpha, value, field_size, field_offset);
}

uint64_t csr_toy_core_alpha_get_field_b(CSR_TOY_CORE_ALPHA alpha) {
    uint64_t field_size = 8;
    uint64_t field_offset = 16;
    return get_field(alpha, field_size, field_offset);
}

CSR_TOY_CORE_ALPHA csr_toy_core_alpha_set_field_c(CSR_TOY_CORE_ALPHA alpha, uint64_t value) {
    uint64_t field_size = 32;
    uint64_t field_offset = 24;
    return set_field(alpha, value, field_size, field_offset);
}

uint64_t csr_toy_core_alpha_get_field_c(CSR_TOY_CORE_ALPHA alpha) {
    uint64_t field_size = 32;
    uint64_t field_offset = 24;
    return get_field(alpha, field_size, field_offset);
}
