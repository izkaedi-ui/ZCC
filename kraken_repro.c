/* Kraken ML Bug Isolation — Minimal Reproduction
 * Tests: double mul, accumulation, exp(), sigmoid, comparison
 * If ZCC produces wrong results here, we've found the codegen fault.
 */
double exp(double x);
#include <stdio.h>

static double relu(double x) { return (x > 0.0) ? x : 0.0; }
static double sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }

/* Test 1: Does double comparison with 0.5 work? */
void test_compare(void) {
    double a = 0.3;
    double b = 0.7;
    printf("TEST1 compare: 0.3 >= 0.5 -> %d (expect 0)\n", (a >= 0.5) ? 1 : 0);
    printf("TEST1 compare: 0.7 >= 0.5 -> %d (expect 1)\n", (b >= 0.5) ? 1 : 0);
}

/* Test 2: Does relu work? */
void test_relu(void) {
    double pos = 2.5;
    double neg = -3.0;
    double r1 = relu(pos);
    double r2 = relu(neg);
    printf("TEST2 relu(2.5)  = %f (expect 2.5)\n", r1);
    printf("TEST2 relu(-3.0) = %f (expect 0.0)\n", r2);
}

/* Test 3: Does sigmoid work? */
void test_sigmoid(void) {
    double s1 = sigmoid(0.0);   /* should be 0.5 */
    double s2 = sigmoid(10.0);  /* should be ~1.0 */
    double s3 = sigmoid(-10.0); /* should be ~0.0 */
    printf("TEST3 sigmoid(0)   = %f (expect 0.500000)\n", s1);
    printf("TEST3 sigmoid(10)  = %f (expect ~1.0)\n", s2);
    printf("TEST3 sigmoid(-10) = %f (expect ~0.0)\n", s3);
}

/* Test 4: Does double multiply-accumulate work? */
void test_mac(void) {
    double a[4] = {1.0, 2.0, 3.0, 4.0};
    double b[4] = {0.1, 0.2, 0.3, 0.4};
    double sum = 0.0;
    int i;
    for (i = 0; i < 4; i++) {
        sum += a[i] * b[i];
    }
    /* 0.1 + 0.4 + 0.9 + 1.6 = 3.0 */
    printf("TEST4 dot product = %f (expect 3.000000)\n", sum);
}

/* Test 5: Does i*32+j index compute correctly for doubles? */
void test_index(void) {
    double w[64]; /* 2x32 matrix */
    int i, j;
    for (i = 0; i < 64; i++) w[i] = (double)i;
    
    double val_0_0 = w[0*32+0];  /* w[0] = 0.0 */
    double val_0_5 = w[0*32+5];  /* w[5] = 5.0 */
    double val_1_0 = w[1*32+0];  /* w[32] = 32.0 */
    double val_1_5 = w[1*32+5];  /* w[37] = 37.0 */
    printf("TEST5 w[0*32+0] = %f (expect 0.0)\n", val_0_0);
    printf("TEST5 w[0*32+5] = %f (expect 5.0)\n", val_0_5);
    printf("TEST5 w[1*32+0] = %f (expect 32.0)\n", val_1_0);
    printf("TEST5 w[1*32+5] = %f (expect 37.0)\n", val_1_5);
}

/* Test 6: Mini classify - the actual Kraken inner loop */
void test_classify(void) {
    double f[4] = {1.0, 0.5, -1.0, 0.0};
    double w1[8] = {0.1, 0.2, -0.1, 0.3,    /* neuron 0 */
                    -0.2, 0.1, 0.4, -0.1};   /* neuron 1 */
    double b1[2] = {0.0, 0.0};
    double w2[2] = {1.0, -1.0};
    double b2[1] = {0.0};

    double h[2];
    int i, j;
    for (i = 0; i < 2; i++) {
        double s = b1[i];
        for (j = 0; j < 4; j++) {
            s += f[j] * w1[i * 4 + j];
        }
        h[i] = relu(s);
    }
    
    double l = b2[0];
    for (i = 0; i < 2; i++) {
        l += h[i] * w2[i];
    }
    
    double sig = sigmoid(l);
    int result = (sig >= 0.5) ? 1 : 0;
    
    printf("TEST6 h[0]=%f h[1]=%f logit=%f sig=%f result=%d\n", 
           h[0], h[1], l, sig, result);
    printf("  (h[0] should be relu(0.1+0.1+0.1+0) = 0.3)\n");
    printf("  (h[1] should be relu(-0.2+0.05-0.4+0) = relu(-0.55) = 0.0)\n");
    printf("  (logit should be 0.3*1.0 + 0.0*(-1.0) = 0.3)\n");
    printf("  (sig should be sigmoid(0.3) ~ 0.574)\n");
    printf("  (result should be 1)\n");
}

int main(void) {
    test_compare();
    printf("---\n");
    test_relu();
    printf("---\n");
    test_sigmoid();
    printf("---\n");
    test_mac();
    printf("---\n");
    test_index();
    printf("---\n");
    test_classify();
    return 0;
}
