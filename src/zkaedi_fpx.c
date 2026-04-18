/*
 * zkaedi_fpx.c — ZKAEDI Novel Floating-Point Formats (Implementation)
 *
 * Strategy: all arithmetic routes through the native double pipeline.
 *   fp24 ops: fp24 -> double -> operate -> double -> fp24
 *   fp48 ops: fp48 -> double -> operate -> double -> fp48
 *
 * IMPORTANT: Uses only 'double' internally, never 'float' locals.
 * ZCC's codegen stores all FP values as 8-byte doubles via movsd.
 * The &float operation returns a 4-byte address but the value was
 * written as 8 bytes — causing misaligned reads. Using double
 * throughout avoids this.
 *
 * (c) 2026 ZKAEDI Systems — v0.10
 */

#include "zkaedi_fpx.h"
#include <stdio.h>
#include <string.h>

/* ================================================================ */
/* INTERNAL: Bit reinterpretation via memcpy                         */
/* ================================================================ */

static unsigned long long dbl_to_bits(double d) {
    unsigned long long i;
    memcpy(&i, &d, 8);
    return i;
}

static double bits_to_dbl(unsigned long long i) {
    double d;
    memcpy(&d, &i, 8);
    return d;
}

/* ================================================================ */
/* fp24_t <-> float/double CONVERSION                                */
/*                                                                   */
/* Work through double -> IEEE double64 bits -> fp24 bits.            */
/* IEEE double64: 1 sign + 11 exp + 52 mantissa (bias 1023)         */
/* IEEE float32:  1 sign + 8 exp  + 23 mantissa (bias 127)          */
/* fp24:          1 sign + 8 exp  + 15 mantissa (bias 127)          */
/*                                                                   */
/* Strategy: extract double bits, convert to float-equivalent exponent*/
/* then truncate mantissa to 15 bits.                                */
/* ================================================================ */

fp24_t fp24_from_float(double f) {
    return fp24_from_double(f);
}

fp24_t fp24_from_double(double d) {
    unsigned long long dbits;
    unsigned int sign, dexp;
    int unbiased_exp;
    unsigned long long dmant;
    unsigned int fexp, fmant;

    dbits = dbl_to_bits(d);
    sign  = (unsigned int)((dbits >> 63) & 1);
    dexp  = (unsigned int)((dbits >> 52) & 0x7FF);
    dmant = dbits & 0xFFFFFFFFFFFFFULL;

    /* Handle special cases */
    if (dexp == 0x7FF) {
        /* Inf or NaN */
        if (dmant == 0) {
            return (fp24_t)((sign << FP24_SIGN_BIT) | FP24_EXP_MASK);
        } else {
            return (fp24_t)((sign << FP24_SIGN_BIT) | FP24_EXP_MASK | 1u);
        }
    }

    if (dexp == 0 && dmant == 0) {
        /* Zero */
        return (fp24_t)(sign << FP24_SIGN_BIT);
    }

    /* Convert double exponent (bias 1023) to float exponent (bias 127) */
    unbiased_exp = (int)dexp - 1023;

    if (unbiased_exp > 127) {
        /* Overflow -> infinity */
        return (fp24_t)((sign << FP24_SIGN_BIT) | FP24_EXP_MASK);
    }

    if (unbiased_exp < -126) {
        /* Underflow -> zero (skip denormals for now) */
        return (fp24_t)(sign << FP24_SIGN_BIT);
    }

    fexp = (unsigned int)(unbiased_exp + 127);

    /* Convert double mantissa (52 bits) to fp24 mantissa (15 bits) */
    /* Drop lower 37 bits */
    fmant = (unsigned int)(dmant >> 37);

    /* Round-to-nearest-even */
    if (dmant & (1ULL << 36)) {
        unsigned long long remainder = dmant & ((1ULL << 36) - 1);
        if (remainder || (fmant & 1)) {
            fmant++;
            if (fmant > FP24_MANT_MASK) {
                fmant = 0;
                fexp++;
                if (fexp >= 0xFF) {
                    return (fp24_t)((sign << FP24_SIGN_BIT) | FP24_EXP_MASK);
                }
            }
        }
    }

    return (fp24_t)((sign << FP24_SIGN_BIT) | (fexp << FP24_MANT_BITS) | fmant);
}

double fp24_to_double(fp24_t x) {
    unsigned int sign, fexp, fmant;
    int unbiased_exp;
    unsigned long long dexp, dmant, dbits;

    sign  = (x >> FP24_SIGN_BIT) & 1;
    fexp  = (x >> FP24_MANT_BITS) & 0xFF;
    fmant = x & FP24_MANT_MASK;

    /* Handle special cases */
    if (fexp == 0xFF) {
        if (fmant == 0) {
            dbits = ((unsigned long long)sign << 63) | (0x7FFULL << 52);
        } else {
            dbits = ((unsigned long long)sign << 63) | (0x7FFULL << 52) | 1ULL;
        }
        return bits_to_dbl(dbits);
    }

    if (fexp == 0 && fmant == 0) {
        dbits = (unsigned long long)sign << 63;
        return bits_to_dbl(dbits);
    }

    /* Convert float exponent (bias 127) to double exponent (bias 1023) */
    unbiased_exp = (int)fexp - 127;
    dexp = (unsigned long long)(unbiased_exp + 1023);

    /* Extend fp24 mantissa (15 bits) to double mantissa (52 bits) */
    dmant = (unsigned long long)fmant << 37;

    dbits = ((unsigned long long)sign << 63) | (dexp << 52) | dmant;
    return bits_to_dbl(dbits);
}

double fp24_to_float(fp24_t x) {
    return fp24_to_double(x);
}

/* ================================================================ */
/* fp48_t <-> double CONVERSION                                      */
/*                                                                   */
/* IEEE double64: 1 sign + 11 exp + 52 mantissa                     */
/* fp48:          1 sign + 11 exp + 36 mantissa                     */
/*                                                                   */
/* Same exponent, truncate/extend mantissa by 16 bits.               */
/* ================================================================ */

fp48_t fp48_from_double(double d) {
    unsigned long long dbits;
    unsigned long long sign, exp, mant;

    dbits = dbl_to_bits(d);
    sign = (dbits >> 63) & 1;
    exp  = (dbits >> 52) & 0x7FF;
    mant = dbits & 0xFFFFFFFFFFFFFULL;

    /* Truncate mantissa: 52 bits -> 36 bits (drop lower 16) */
    if (mant & 0x8000ULL) {
        if ((mant & 0x7FFFULL) || (mant & 0x10000ULL)) {
            mant = (mant >> 16) + 1;
            if (mant > FP48_MANT_MASK) {
                mant = 0;
                exp++;
                if (exp >= 0x7FF) {
                    return (fp48_t)((sign << FP48_SIGN_BIT) | FP48_EXP_MASK);
                }
            }
        } else {
            mant = mant >> 16;
        }
    } else {
        mant = mant >> 16;
    }

    return (fp48_t)((sign << FP48_SIGN_BIT) | (exp << FP48_MANT_BITS) | mant);
}

double fp48_to_double(fp48_t x) {
    unsigned long long sign, exp, mant, dbits;

    sign = (x >> FP48_SIGN_BIT) & 1;
    exp  = (x >> FP48_MANT_BITS) & 0x7FF;
    mant = x & FP48_MANT_MASK;

    /* Extend mantissa: 36 bits -> 52 bits */
    dbits = (sign << 63) | (exp << 52) | (mant << 16);
    return bits_to_dbl(dbits);
}

fp48_t fp48_from_float(double f) {
    return fp48_from_double(f);
}

double fp48_to_float(fp48_t x) {
    return fp48_to_double(x);
}

/* ================================================================ */
/* ARITHMETIC: fp24_t (via double round-trip)                        */
/* ================================================================ */

fp24_t fp24_add(fp24_t a, fp24_t b) {
    return fp24_from_double(fp24_to_double(a) + fp24_to_double(b));
}

fp24_t fp24_sub(fp24_t a, fp24_t b) {
    return fp24_from_double(fp24_to_double(a) - fp24_to_double(b));
}

fp24_t fp24_mul(fp24_t a, fp24_t b) {
    return fp24_from_double(fp24_to_double(a) * fp24_to_double(b));
}

fp24_t fp24_div(fp24_t a, fp24_t b) {
    return fp24_from_double(fp24_to_double(a) / fp24_to_double(b));
}

fp24_t fp24_neg(fp24_t a) {
    return a ^ FP24_SIGN_MASK;
}

fp24_t fp24_abs(fp24_t a) {
    return a & ~FP24_SIGN_MASK;
}

/* ================================================================ */
/* ARITHMETIC: fp48_t (via double round-trip)                        */
/* ================================================================ */

fp48_t fp48_add(fp48_t a, fp48_t b) {
    return fp48_from_double(fp48_to_double(a) + fp48_to_double(b));
}

fp48_t fp48_sub(fp48_t a, fp48_t b) {
    return fp48_from_double(fp48_to_double(a) - fp48_to_double(b));
}

fp48_t fp48_mul(fp48_t a, fp48_t b) {
    return fp48_from_double(fp48_to_double(a) * fp48_to_double(b));
}

fp48_t fp48_div(fp48_t a, fp48_t b) {
    return fp48_from_double(fp48_to_double(a) / fp48_to_double(b));
}

fp48_t fp48_neg(fp48_t a) {
    return a ^ FP48_SIGN_MASK;
}

fp48_t fp48_abs(fp48_t a) {
    return a & ~FP48_SIGN_MASK;
}

/* ================================================================ */
/* COMPARISON                                                        */
/* ================================================================ */

int fp24_isnan(fp24_t a) {
    unsigned int exp  = (a & FP24_EXP_MASK)  >> FP24_MANT_BITS;
    unsigned int mant = a & FP24_MANT_MASK;
    return (exp == 0xFF) && (mant != 0);
}

int fp24_isinf(fp24_t a) {
    unsigned int exp  = (a & FP24_EXP_MASK)  >> FP24_MANT_BITS;
    unsigned int mant = a & FP24_MANT_MASK;
    return (exp == 0xFF) && (mant == 0);
}

int fp24_iszero(fp24_t a) {
    return (a & ~FP24_SIGN_MASK) == 0;
}

int fp24_eq(fp24_t a, fp24_t b) {
    if (fp24_isnan(a) || fp24_isnan(b)) return 0;
    if (fp24_iszero(a) && fp24_iszero(b)) return 1;
    return a == b;
}

int fp24_lt(fp24_t a, fp24_t b) {
    return fp24_to_double(a) < fp24_to_double(b);
}

int fp24_le(fp24_t a, fp24_t b) {
    return fp24_to_double(a) <= fp24_to_double(b);
}

int fp24_gt(fp24_t a, fp24_t b) {
    return fp24_to_double(a) > fp24_to_double(b);
}

int fp24_ge(fp24_t a, fp24_t b) {
    return fp24_to_double(a) >= fp24_to_double(b);
}

int fp48_isnan(fp48_t a) {
    unsigned long long exp  = (a & FP48_EXP_MASK) >> FP48_MANT_BITS;
    unsigned long long mant = a & FP48_MANT_MASK;
    return (exp == 0x7FF) && (mant != 0);
}

int fp48_isinf(fp48_t a) {
    unsigned long long exp  = (a & FP48_EXP_MASK) >> FP48_MANT_BITS;
    unsigned long long mant = a & FP48_MANT_MASK;
    return (exp == 0x7FF) && (mant == 0);
}

int fp48_iszero(fp48_t a) {
    return (a & ~FP48_SIGN_MASK) == 0;
}

int fp48_eq(fp48_t a, fp48_t b) {
    if (fp48_isnan(a) || fp48_isnan(b)) return 0;
    if (fp48_iszero(a) && fp48_iszero(b)) return 1;
    return a == b;
}

int fp48_lt(fp48_t a, fp48_t b) {
    return fp48_to_double(a) < fp48_to_double(b);
}

int fp48_le(fp48_t a, fp48_t b) {
    return fp48_to_double(a) <= fp48_to_double(b);
}

int fp48_gt(fp48_t a, fp48_t b) {
    return fp48_to_double(a) > fp48_to_double(b);
}

int fp48_ge(fp48_t a, fp48_t b) {
    return fp48_to_double(a) >= fp48_to_double(b);
}

/* ================================================================ */
/* CROSS-FORMAT CONVERSION                                           */
/* ================================================================ */

fp48_t fp24_to_fp48(fp24_t a) {
    return fp48_from_double(fp24_to_double(a));
}

fp24_t fp48_to_fp24(fp48_t a) {
    return fp24_from_double(fp48_to_double(a));
}

/* ================================================================ */
/* UTILITY                                                           */
/* ================================================================ */

void fp24_print(fp24_t a) {
    unsigned int sign = (a >> FP24_SIGN_BIT) & 1;
    unsigned int exp  = (a >> FP24_MANT_BITS) & 0xFF;
    unsigned int mant = a & FP24_MANT_MASK;

    if (fp24_isnan(a)) {
        printf("fp24(NaN)");
    } else if (fp24_isinf(a)) {
        printf("fp24(%cInf)", sign ? '-' : '+');
    } else {
        printf("fp24(bits=0x%06X  s=%u e=%u m=0x%04X  val=%.7g)",
               a & FP24_FULL_MASK,
               sign, exp, mant,
               fp24_to_double(a));
    }
}

void fp48_print(fp48_t a) {
    unsigned long long sign = (a >> FP48_SIGN_BIT) & 1;
    unsigned long long exp  = (a >> FP48_MANT_BITS) & 0x7FF;
    unsigned long long mant = a & FP48_MANT_MASK;

    if (fp48_isnan(a)) {
        printf("fp48(NaN)");
    } else if (fp48_isinf(a)) {
        printf("fp48(%cInf)", sign ? '-' : '+');
    } else {
        printf("fp48(bits=0x%012llX  s=%llu e=%llu m=0x%09llX  val=%.15g)",
               a & FP48_FULL_MASK,
               sign, exp, mant,
               fp48_to_double(a));
    }
}

unsigned int fp24_raw(fp24_t a) {
    return a & FP24_FULL_MASK;
}

unsigned long long fp48_raw(fp48_t a) {
    return a & FP48_FULL_MASK;
}
