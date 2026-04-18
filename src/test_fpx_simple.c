/* Minimal ZCC + fpx test — using double literals only */
#include "zkaedi_fpx.h"
#include "zkaedi_fpx.c"
#include <stdio.h>

int main(void) {
    fp24_t a;
    fp24_t b;
    fp24_t c;
    double fa;
    double fb;
    double fc;

    a = fp24_from_double(2.5);
    fa = fp24_to_double(a);
    printf("fp24(2.5) = %f  bits=0x%X\n", fa, a);

    b = fp24_from_double(3.5);
    fb = fp24_to_double(b);
    printf("fp24(3.5) = %f  bits=0x%X\n", fb, b);

    c = fp24_add(a, b);
    fc = fp24_to_double(c);
    printf("2.5 + 3.5 = %f  bits=0x%X\n", fc, c);

    printf("isnan(a) = %d\n", fp24_isnan(a));

    printf("[OK] simple fpx test\n");
    return 0;
}
