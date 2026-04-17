/* Kraken Bug Isolation Phase 2: Large const array + pointer argument passing
 * The core ops are fine. Test the actual data path from global const arrays
 * through pointer arguments into classify_binary.
 */
double exp(double x);
#include <stdio.h>

static double relu(double x) { return (x > 0.0) ? x : 0.0; }
static double sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }

/* Simulate a 32x32 weight matrix (1024 doubles) + bias (32 doubles) */
static const double test_w1[1024] = {
    /* Row 0: all 0.1 */
    0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
    0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
    /* Rows 1-31: all 0.0 (zero-initialized) */
};
static const double test_b1[32] = {0};
static const double test_w2[32] = {1.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const double test_b2[1] = {0.0};

/* Negative bias to force SAFE */
static const double safe_w1[1024] = {0};
static const double safe_b1[32] = {-10.0, -10.0, -10.0, -10.0, -10.0, -10.0, -10.0, -10.0,
                                    -10.0, -10.0, -10.0, -10.0, -10.0, -10.0, -10.0, -10.0,
                                    -10.0, -10.0, -10.0, -10.0, -10.0, -10.0, -10.0, -10.0,
                                    -10.0, -10.0, -10.0, -10.0, -10.0, -10.0, -10.0, -10.0};
static const double safe_w2[32] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
static const double safe_b2[1] = {-5.0};

static int classify_binary(const double *f, const double *w1, const double *b1, const double *w2, const double *b2) {
    double h[32]; int i, j;
    for (i = 0; i < 32; i++) {
        double s = b1[i];
        for (j = 0; j < 32; j++) s += f[j] * w1[i * 32 + j];
        h[i] = relu(s);
    }
    double l = b2[0];
    for (i = 0; i < 32; i++) l += h[i] * w2[i];
    double sig = sigmoid(l);
    printf("  [debug] logit=%f sigmoid=%f\n", l, sig);
    return (sig >= 0.5) ? 1 : 0;
}

int main(void) {
    double features[32] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                           1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                           1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                           1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    
    printf("=== VULN weights (expect VULNERABLE) ===\n");
    int r1 = classify_binary(features, test_w1, test_b1, test_w2, test_b2);
    printf("Result: %s\n\n", r1 ? "VULNERABLE" : "SAFE");
    
    printf("=== SAFE weights (expect SAFE) ===\n");
    int r2 = classify_binary(features, safe_w1, safe_b1, safe_w2, safe_b2);
    printf("Result: %s\n\n", r2 ? "VULNERABLE" : "SAFE");

    double clean[32] = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01,
                        0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01,
                        0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01,
                        0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};
    printf("=== SAFE weights + clean features (expect SAFE) ===\n");
    int r3 = classify_binary(clean, safe_w1, safe_b1, safe_w2, safe_b2);
    printf("Result: %s\n", r3 ? "VULNERABLE" : "SAFE");
    
    return 0;
}
