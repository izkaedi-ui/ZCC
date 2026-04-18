/*
 * zkaedi_fpx.c — ZKAEDI Novel Floating-Point Formats (Clean Implementation)
 *
 * Requires ZCC >= v0.10.1 (CG-FLOAT-001 fix).
 * Uses memcpy for float<->int bit reinterpretation.
 *
 * fp24 ops route through float: fp24 -> float -> operate -> float -> fp24
 * fp48 ops route through double: fp48 -> double -> operate -> double -> fp48
 *
 * (c) 2026 ZKAEDI Systems — v0.10.2
 */

#include "zkaedi_fpx.h"
#include <stdio.h>
#include <string.h>

/* ================================================================ */
/* INTERNAL: Bit reinterpretation via memcpy                         */
/* ================================================================ */

static unsigned int float_to_bits(float f) {
    unsigned int i;
    memcpy(&i, &f, 4);
    return i;
}

static float bits_to_float(unsigned int i) {
    float f;
    memcpy(&f, &i, 4);
    return f;
}

static unsigned long long double_to_bits(double d) {
    unsigned long long i;
    memcpy(&i, &d, 8);
    return i;
}

static double bits_to_double(unsigned long long i) {
    double d;
    memcpy(&d, &i, 8);
    return d;
}

/* ================================================================ */
/* fp24_t <-> float/double CONVERSION                                */
/*                                                                   */
/* All conversions route through double bits internally.             */
/* ZCC has a known bug (CG-FLOAT-002) where float function params   */
/* arrive as zero. By working through double, we avoid this.        */
/*                                                                   */
/* double64: 1s + 11exp(bias 1023) + 52mant                         */
/* fp24:     1s + 8exp(bias 127)   + 15mant                         */
/* ================================================================ */

fp24_t fp24_from_double(double d) {
    unsigned long long dbits;
    unsigned int sign, fexp, fmant;
    int unbiased;
    unsigned long long dmant;

    dbits = double_to_bits(d);
    sign  = (unsigned int)((dbits >> 63) & 1);

    /* Special cases */
    if (((dbits >> 52) & 0x7FF) == 0x7FF) {
        if (dbits & 0xFFFFFFFFFFFFFULL)
            return (fp24_t)((sign << FP24_SIGN_BIT) | FP24_EXP_MASK | 1u);  /* NaN */
        return (fp24_t)((sign << FP24_SIGN_BIT) | FP24_EXP_MASK);          /* Inf */
    }
    if (((dbits >> 52) & 0x7FF) == 0 && (dbits & 0xFFFFFFFFFFFFFULL) == 0)
        return (fp24_t)(sign << FP24_SIGN_BIT);  /* Zero */

    unbiased = (int)((dbits >> 52) & 0x7FF) - 1023;
    if (unbiased > 127)  return (fp24_t)((sign << FP24_SIGN_BIT) | FP24_EXP_MASK);
    if (unbiased < -126) return (fp24_t)(sign << FP24_SIGN_BIT);

    fexp  = (unsigned int)(unbiased + 127);
    dmant = dbits & 0xFFFFFFFFFFFFFULL;

    /* 52-bit mantissa -> 15-bit: drop lower 37 bits */
    fmant = (unsigned int)(dmant >> 37);

    /* Round-to-nearest-even */
    if (dmant & (1ULL << 36)) {
        unsigned long long rem = dmant & ((1ULL << 36) - 1);
        if (rem || (fmant & 1)) {
            fmant++;
            if (fmant > FP24_MANT_MASK) {
                fmant = 0;
                fexp++;
                if (fexp >= 0xFF)
                    return (fp24_t)((sign << FP24_SIGN_BIT) | FP24_EXP_MASK);
            }
        }
    }

    return (fp24_t)((sign << FP24_SIGN_BIT) | (fexp << FP24_MANT_BITS) | fmant);
}

fp24_t fp24_from_float(float f) {
    return fp24_from_double((double)f);
}

double fp24_to_double(fp24_t x) {
    unsigned int sign, fexp, fmant;
    unsigned long long dexp, dmant, dbits;

    sign  = (x >> FP24_SIGN_BIT) & 1;
    fexp  = (x >> FP24_MANT_BITS) & 0xFF;
    fmant = x & FP24_MANT_MASK;

    if (fexp == 0xFF) {
        dbits = ((unsigned long long)sign << 63) | (0x7FFULL << 52);
        if (fmant) dbits |= 1ULL;
        return bits_to_double(dbits);
    }
    /* CG-FPX-FTZ-001: flush subnormal fp24 bit patterns to ±0.0.
     * exp==0, mant!=0 is a subnormal. We FTZ on both sides for consistency.
     * These bit patterns are unreachable via fp24_from_double. */
    if (fexp == 0)
        return bits_to_double((unsigned long long)sign << 63);

    dexp  = (unsigned long long)((int)fexp - 127 + 1023);
    dmant = (unsigned long long)fmant << 37;
    dbits = ((unsigned long long)sign << 63) | (dexp << 52) | dmant;
    return bits_to_double(dbits);
}

float fp24_to_float(fp24_t x) {
    return (float)fp24_to_double(x);
}

/* ================================================================ */
/* fp48_t <-> double CONVERSION                                      */
/* ================================================================ */

fp48_t fp48_from_double(double d) {
    unsigned long long u, sign, exp, mant;

    u = double_to_bits(d);
    sign = (u >> 63) & 1;
    exp  = (u >> 52) & 0x7FF;
    mant = u & 0xFFFFFFFFFFFFFULL;

    /* Round-to-nearest-even: 52-bit mantissa -> 36-bit */
    if (mant & 0x8000ULL) {
        if ((mant & 0x7FFFULL) || (mant & 0x10000ULL)) {
            mant = (mant >> 16) + 1;
            if (mant > FP48_MANT_MASK) {
                mant = 0;
                exp++;
                if (exp >= 0x7FF)
                    return (fp48_t)((sign << FP48_SIGN_BIT) | FP48_EXP_MASK);
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
    unsigned long long sign, exp, mant, u;
    sign = (x >> FP48_SIGN_BIT) & 1;
    exp  = (x >> FP48_MANT_BITS) & 0x7FF;
    mant = x & FP48_MANT_MASK;
    u = (sign << 63) | (exp << 52) | (mant << 16);
    return bits_to_double(u);
}

fp48_t fp48_from_float(float f) {
    return fp48_from_double((double)f);
}

float fp48_to_float(fp48_t x) {
    return (float)fp48_to_double(x);
}

/* ================================================================ */
/* ARITHMETIC: fp24_t                                                */
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
fp24_t fp24_neg(fp24_t a) { return a ^ FP24_SIGN_MASK; }
fp24_t fp24_abs(fp24_t a) { return a & ~FP24_SIGN_MASK; }

/* ================================================================ */
/* ARITHMETIC: fp48_t                                                */
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
fp48_t fp48_neg(fp48_t a) { return a ^ FP48_SIGN_MASK; }
fp48_t fp48_abs(fp48_t a) { return a & ~FP48_SIGN_MASK; }

/* ================================================================ */
/* COMPARISON                                                        */
/* ================================================================ */

int fp24_isnan(fp24_t a) {
    return ((a & FP24_EXP_MASK) >> FP24_MANT_BITS) == 0xFF && (a & FP24_MANT_MASK);
}
int fp24_isinf(fp24_t a) {
    return ((a & FP24_EXP_MASK) >> FP24_MANT_BITS) == 0xFF && !(a & FP24_MANT_MASK);
}
int fp24_iszero(fp24_t a) { return (a & ~FP24_SIGN_MASK) == 0; }
int fp24_eq(fp24_t a, fp24_t b) {
    if (fp24_isnan(a) || fp24_isnan(b)) return 0;
    if (fp24_iszero(a) && fp24_iszero(b)) return 1;
    return a == b;
}
int fp24_lt(fp24_t a, fp24_t b) { return fp24_to_double(a) < fp24_to_double(b); }
int fp24_le(fp24_t a, fp24_t b) { return fp24_to_double(a) <= fp24_to_double(b); }
int fp24_gt(fp24_t a, fp24_t b) { return fp24_to_double(a) > fp24_to_double(b); }
int fp24_ge(fp24_t a, fp24_t b) { return fp24_to_double(a) >= fp24_to_double(b); }

int fp48_isnan(fp48_t a) {
    return ((a & FP48_EXP_MASK) >> FP48_MANT_BITS) == 0x7FF && (a & FP48_MANT_MASK);
}
int fp48_isinf(fp48_t a) {
    return ((a & FP48_EXP_MASK) >> FP48_MANT_BITS) == 0x7FF && !(a & FP48_MANT_MASK);
}
int fp48_iszero(fp48_t a) { return (a & ~FP48_SIGN_MASK) == 0; }
int fp48_eq(fp48_t a, fp48_t b) {
    if (fp48_isnan(a) || fp48_isnan(b)) return 0;
    if (fp48_iszero(a) && fp48_iszero(b)) return 1;
    return a == b;
}
int fp48_lt(fp48_t a, fp48_t b) { return fp48_to_double(a) < fp48_to_double(b); }
int fp48_le(fp48_t a, fp48_t b) { return fp48_to_double(a) <= fp48_to_double(b); }
int fp48_gt(fp48_t a, fp48_t b) { return fp48_to_double(a) > fp48_to_double(b); }
int fp48_ge(fp48_t a, fp48_t b) { return fp48_to_double(a) >= fp48_to_double(b); }

/* ================================================================ */
/* CROSS-FORMAT                                                      */
/* ================================================================ */

fp48_t fp24_to_fp48(fp24_t a) { return fp48_from_double(fp24_to_double(a)); }
fp24_t fp48_to_fp24(fp48_t a) { return fp24_from_double(fp48_to_double(a)); }

/* ================================================================ */
/* UTILITY                                                           */
/* ================================================================ */

void fp24_print(fp24_t a) {
    unsigned int s = (a >> FP24_SIGN_BIT) & 1;
    unsigned int e = (a >> FP24_MANT_BITS) & 0xFF;
    unsigned int m = a & FP24_MANT_MASK;
    if (fp24_isnan(a)) printf("fp24(NaN)");
    else if (fp24_isinf(a)) printf("fp24(%cInf)", s ? '-' : '+');
    else printf("fp24(bits=0x%06X s=%u e=%u m=0x%04X val=%.7g)",
                a & FP24_FULL_MASK, s, e, m, fp24_to_double(a));
}

void fp48_print(fp48_t a) {
    unsigned long long s = (a >> FP48_SIGN_BIT) & 1;
    unsigned long long e = (a >> FP48_MANT_BITS) & 0x7FF;
    unsigned long long m = a & FP48_MANT_MASK;
    if (fp48_isnan(a)) printf("fp48(NaN)");
    else if (fp48_isinf(a)) printf("fp48(%cInf)", s ? '-' : '+');
    else printf("fp48(bits=0x%012llX s=%llu e=%llu m=0x%09llX val=%.15g)",
                a & FP48_FULL_MASK, s, e, m, fp48_to_double(a));
}

unsigned int       fp24_raw(fp24_t a) { return a & FP24_FULL_MASK; }
unsigned long long fp48_raw(fp48_t a) { return a & FP48_FULL_MASK; }
