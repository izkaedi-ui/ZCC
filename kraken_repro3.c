/* Phase 3: Isolate const array pointer dereference for large arrays */
#include <stdio.h>

static const double big_arr[1024] = {
    /* First 32 elements: 0.1 each */
    0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
    0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
    /* Rest is zero */
};

/* Direct access: works? */
void test_direct(void) {
    printf("direct big_arr[0] = %f (expect 0.1)\n", big_arr[0]);
    printf("direct big_arr[31] = %f (expect 0.1)\n", big_arr[31]);
    printf("direct big_arr[32] = %f (expect 0.0)\n", big_arr[32]);
}

/* Pointer access: works? */
void test_ptr(const double *p) {
    printf("ptr p[0] = %f (expect 0.1)\n", p[0]);
    printf("ptr p[31] = %f (expect 0.1)\n", p[31]);
    printf("ptr p[32] = %f (expect 0.0)\n", p[32]);
}

/* Pointer + computed index: works? */
void test_computed(const double *p) {
    int i = 0;
    int j = 5;
    printf("ptr p[0*32+5] = %f (expect 0.1)\n", p[i*32+j]);
    i = 1; j = 0;
    printf("ptr p[1*32+0] = %f (expect 0.0)\n", p[i*32+j]);
}

/* Accumulation via pointer: works? */
void test_accum(const double *p) {
    double sum = 0.0;
    int j;
    for (j = 0; j < 32; j++) {
        sum += p[j];
    }
    printf("accum sum of first 32 = %f (expect 3.2)\n", sum);
}

/* Multiply-accumulate via two pointers: works? */
void test_mac(const double *f, const double *w) {
    double sum = 0.0;
    int j;
    for (j = 0; j < 32; j++) {
        sum += f[j] * w[j];
    }
    printf("mac dot(f,w[0..31]) = %f (expect 3.2)\n", sum);
}

int main(void) {
    double feat[32];
    int i;
    for (i = 0; i < 32; i++) feat[i] = 1.0;
    
    test_direct();
    printf("---\n");
    test_ptr(big_arr);
    printf("---\n");
    test_computed(big_arr);
    printf("---\n");
    test_accum(big_arr);
    printf("---\n");
    test_mac(feat, big_arr);
    return 0;
}
