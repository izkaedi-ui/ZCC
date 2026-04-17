/* Phase 5: Is it static const, const, or all double arrays? */
#include <stdio.h>

static const double sc_arr[4] = {1.1, 2.2, 3.3, 4.4};
const double c_arr[4] = {5.5, 6.6, 7.7, 8.8};
double mut_arr[4] = {9.9, 10.1, 11.2, 12.3};
static const int sc_int[4] = {10, 20, 30, 40};
static const float sc_float[4] = {1.5, 2.5, 3.5, 4.5};

int main(void) {
    printf("static const double[0] = %f (expect 1.1)\n", sc_arr[0]);
    printf("const double[0]        = %f (expect 5.5)\n", c_arr[0]);
    printf("mutable double[0]      = %f (expect 9.9)\n", mut_arr[0]);
    printf("static const int[0]    = %d (expect 10)\n", sc_int[0]);
    printf("static const float[0]  = %f (expect 1.5)\n", (double)sc_float[0]);
    return 0;
}
