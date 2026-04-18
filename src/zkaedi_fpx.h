/*
 * zkaedi_fpx.h — ZKAEDI Novel Floating-Point Formats
 *
 * fp24_t: 24-bit float (1 sign + 8 exp + 15 mantissa)
 *   - Same exponent range as IEEE 754 float32
 *   - ~4.5 decimal digits of precision (vs float's ~7.2)
 *   - Stored in a uint32_t for alignment (upper 8 bits unused)
 *
 * fp48_t: 48-bit float (1 sign + 11 exp + 36 mantissa)
 *   - Same exponent range as IEEE 754 double64
 *   - ~10.8 decimal digits of precision (vs double's ~15.9)
 *   - Stored in a uint64_t for alignment (upper 16 bits unused)
 *
 * Design: Option C — opaque struct types, no cross-type casts.
 * All operations go through explicit function calls.
 *
 * (c) 2026 ZKAEDI Systems — v0.10
 */

#ifndef ZKAEDI_FPX_H
#define ZKAEDI_FPX_H

/* ================================================================ */
/* TYPE DEFINITIONS                                                  */
/* ================================================================ */

/* fp24_t: 24-bit float packed into lower 24 bits of a 32-bit word.
 * Bit layout: [23] sign | [22:15] exponent (8-bit, bias 127) | [14:0] mantissa
 * Note: typedef (not struct) for ZCC ABI compatibility — returned in %eax. */
typedef unsigned int fp24_t;

/* fp48_t: 48-bit float packed into lower 48 bits of a 64-bit word.
 * Bit layout: [47] sign | [46:36] exponent (11-bit, bias 1023) | [35:0] mantissa
 * Note: typedef (not struct) for ZCC ABI compatibility — returned in %rax. */
typedef unsigned long long fp48_t;

/* ================================================================ */
/* FORMAT CONSTANTS                                                  */
/* ================================================================ */

#define FP24_SIGN_BIT    23
#define FP24_EXP_BITS    8
#define FP24_MANT_BITS   15
#define FP24_EXP_BIAS    127
#define FP24_EXP_MASK    0x7F8000u   /* bits [22:15] */
#define FP24_MANT_MASK   0x007FFFu   /* bits [14:0]  */
#define FP24_SIGN_MASK   0x800000u   /* bit  [23]    */
#define FP24_FULL_MASK   0xFFFFFFu   /* all 24 bits  */

#define FP48_SIGN_BIT    47
#define FP48_EXP_BITS    11
#define FP48_MANT_BITS   36
#define FP48_EXP_BIAS    1023
#define FP48_EXP_MASK    0x7FF000000000ULL  /* bits [46:36] */
#define FP48_MANT_MASK   0x000FFFFFFFFFULL  /* bits [35:0]  */
#define FP48_SIGN_MASK   0x800000000000ULL  /* bit  [47]    */
#define FP48_FULL_MASK   0xFFFFFFFFFFFFULL  /* all 48 bits  */

#define FP24_ZERO        ((fp24_t)0)
#define FP24_NEG_ZERO    ((fp24_t)FP24_SIGN_MASK)
#define FP24_INF         ((fp24_t)FP24_EXP_MASK)
#define FP24_NEG_INF     ((fp24_t)(FP24_SIGN_MASK | FP24_EXP_MASK))
#define FP24_NAN         ((fp24_t)(FP24_EXP_MASK | 1u))

#define FP48_ZERO        ((fp48_t)0)
#define FP48_NEG_ZERO    ((fp48_t)FP48_SIGN_MASK)
#define FP48_INF         ((fp48_t)FP48_EXP_MASK)
#define FP48_NEG_INF     ((fp48_t)(FP48_SIGN_MASK | FP48_EXP_MASK))
#define FP48_NAN         ((fp48_t)(FP48_EXP_MASK | 1ULL))

/* ================================================================ */
/* CONVERSION: fp24_t <-> float                                      */
/* ================================================================ */

/* NOTE: All functions use 'double' in signatures because ZCC's
 * float parameter passing is broken (float params arrive as zero).
 * Use fp24_from_float(2.5) instead of fp24_from_float(2.5f). */

fp24_t fp24_from_float(double f);
double fp24_to_float(fp24_t x);

fp24_t fp24_from_double(double d);
double fp24_to_double(fp24_t x);

/* ================================================================ */
/* CONVERSION: fp48_t <-> double                                     */
/* ================================================================ */

fp48_t fp48_from_double(double d);
double fp48_to_double(fp48_t x);

fp48_t fp48_from_float(double f);
double fp48_to_float(fp48_t x);

/* ================================================================ */
/* ARITHMETIC: fp24_t                                                */
/* ================================================================ */

fp24_t fp24_add(fp24_t a, fp24_t b);
fp24_t fp24_sub(fp24_t a, fp24_t b);
fp24_t fp24_mul(fp24_t a, fp24_t b);
fp24_t fp24_div(fp24_t a, fp24_t b);
fp24_t fp24_neg(fp24_t a);
fp24_t fp24_abs(fp24_t a);

/* ================================================================ */
/* ARITHMETIC: fp48_t                                                */
/* ================================================================ */

fp48_t fp48_add(fp48_t a, fp48_t b);
fp48_t fp48_sub(fp48_t a, fp48_t b);
fp48_t fp48_mul(fp48_t a, fp48_t b);
fp48_t fp48_div(fp48_t a, fp48_t b);
fp48_t fp48_neg(fp48_t a);
fp48_t fp48_abs(fp48_t a);

/* ================================================================ */
/* COMPARISON                                                        */
/* ================================================================ */

int fp24_eq(fp24_t a, fp24_t b);
int fp24_lt(fp24_t a, fp24_t b);
int fp24_le(fp24_t a, fp24_t b);
int fp24_gt(fp24_t a, fp24_t b);
int fp24_ge(fp24_t a, fp24_t b);
int fp24_isnan(fp24_t a);
int fp24_isinf(fp24_t a);
int fp24_iszero(fp24_t a);

int fp48_eq(fp48_t a, fp48_t b);
int fp48_lt(fp48_t a, fp48_t b);
int fp48_le(fp48_t a, fp48_t b);
int fp48_gt(fp48_t a, fp48_t b);
int fp48_ge(fp48_t a, fp48_t b);
int fp48_isnan(fp48_t a);
int fp48_isinf(fp48_t a);
int fp48_iszero(fp48_t a);

/* ================================================================ */
/* CROSS-FORMAT CONVERSION (explicit only — no implicit casts)       */
/* ================================================================ */

fp48_t fp24_to_fp48(fp24_t a);   /* widen: lossless */
fp24_t fp48_to_fp24(fp48_t a);   /* narrow: lossy   */

/* ================================================================ */
/* UTILITY                                                           */
/* ================================================================ */

/* Print formatted representation */
void fp24_print(fp24_t a);
void fp48_print(fp48_t a);

/* Raw bit access */
unsigned int       fp24_raw(fp24_t a);
unsigned long long fp48_raw(fp48_t a);

#endif /* ZKAEDI_FPX_H */
