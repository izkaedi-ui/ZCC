/*
 * zkaedi_fpx.h — ZKAEDI Novel Floating-Point Formats
 *
 * fp24_t: 24-bit float (1 sign + 8 exp + 15 mantissa)
 *   - Same exponent range as IEEE 754 float32
 *   - ~4.5 decimal digits of precision (vs float's ~7.2)
 *
 * fp48_t: 48-bit float (1 sign + 11 exp + 36 mantissa)
 *   - Same exponent range as IEEE 754 double64
 *   - ~10.8 decimal digits of precision (vs double's ~15.9)
 *
 * Design: Option C — opaque typedef types, no cross-type casts.
 * All operations go through explicit function calls.
 * Requires ZCC >= v0.10.1 (CG-FLOAT-001 fix).
 *
 * Subnormal handling: fp24_t and fp48_t are flush-to-zero (FTZ) on both
 * conversion boundaries. This is a deliberate design choice, not a bug.
 *
 *   fp24_from_double(d): any |d| < 2^-126 (smallest normal fp24) returns
 *     the signed zero with the sign of d. Gradual underflow NOT implemented.
 *
 *   fp24_to_double(u): bit patterns with biased exponent == 0 (subnormals)
 *     return +/-0.0 regardless of mantissa. These patterns are unreachable
 *     via fp24_from_double and are defined as equivalent to signed zero.
 *
 *   Callers requiring IEEE 754 gradual underflow must post-process, use
 *   fp48_t, or request a future fp24_denorm build variant.
 *
 * (c) 2026 ZKAEDI Systems — v0.10.2
 */

#ifndef ZKAEDI_FPX_H
#define ZKAEDI_FPX_H

/* ================================================================ */
/* TYPE DEFINITIONS                                                  */
/* ================================================================ */

typedef unsigned int       fp24_t;
typedef unsigned long long fp48_t;

/* ================================================================ */
/* FORMAT CONSTANTS                                                  */
/* ================================================================ */

#define FP24_SIGN_BIT    23
#define FP24_EXP_BITS    8
#define FP24_MANT_BITS   15
#define FP24_EXP_BIAS    127
#define FP24_EXP_MASK    0x7F8000u
#define FP24_MANT_MASK   0x007FFFu
#define FP24_SIGN_MASK   0x800000u
#define FP24_FULL_MASK   0xFFFFFFu

#define FP48_SIGN_BIT    47
#define FP48_EXP_BITS    11
#define FP48_MANT_BITS   36
#define FP48_EXP_BIAS    1023
#define FP48_EXP_MASK    0x7FF000000000ULL
#define FP48_MANT_MASK   0x000FFFFFFFFFULL
#define FP48_SIGN_MASK   0x800000000000ULL
#define FP48_FULL_MASK   0xFFFFFFFFFFFFULL

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
/* CONVERSION                                                        */
/* ================================================================ */

fp24_t fp24_from_float(float f);
float  fp24_to_float(fp24_t x);
fp24_t fp24_from_double(double d);
double fp24_to_double(fp24_t x);

fp48_t fp48_from_double(double d);
double fp48_to_double(fp48_t x);
fp48_t fp48_from_float(float f);
float  fp48_to_float(fp48_t x);

/* ================================================================ */
/* ARITHMETIC                                                        */
/* ================================================================ */

fp24_t fp24_add(fp24_t a, fp24_t b);
fp24_t fp24_sub(fp24_t a, fp24_t b);
fp24_t fp24_mul(fp24_t a, fp24_t b);
fp24_t fp24_div(fp24_t a, fp24_t b);
fp24_t fp24_neg(fp24_t a);
fp24_t fp24_abs(fp24_t a);

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
/* CROSS-FORMAT CONVERSION                                           */
/* ================================================================ */

fp48_t fp24_to_fp48(fp24_t a);
fp24_t fp48_to_fp24(fp48_t a);

/* ================================================================ */
/* UTILITY                                                           */
/* ================================================================ */

void fp24_print(fp24_t a);
void fp48_print(fp48_t a);
unsigned int       fp24_raw(fp24_t a);
unsigned long long fp48_raw(fp48_t a);

#endif /* ZKAEDI_FPX_H */
