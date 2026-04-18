#include <stdio.h>
int main(void) {
    /* Integer-to-integer narrowing */
    long long ll = 0x123456789ABCDEF0LL;
    int i_from_ll = ll;              printf("ll->int:       0x%X\n", i_from_ll);
    short s_from_ll = ll;            printf("ll->short:     0x%X\n", s_from_ll & 0xFFFF);
    char c_from_ll = ll;             printf("ll->char:      0x%X\n", c_from_ll & 0xFF);

    /* Integer widening to float */
    char c = 42;
    float f_from_c = c;              printf("char->float:   %g\n", (double)f_from_c);
    short s = 1000;
    float f_from_s = s;              printf("short->float:  %g\n", (double)f_from_s);

    /* Integer widening to double */
    char c2 = 42;
    double d_from_c = c2;            printf("char->double:  %g\n", d_from_c);
    short s2 = 1000;
    double d_from_s = s2;            printf("short->double: %g\n", d_from_s);

    /* Large-int to float (precision edge) */
    long li = 0x1FFFFFFFFFFFFFL;     /* 53+ bit value — loses precision as float */
    float f_from_li = li;            printf("long->float:   %g\n", (double)f_from_li);

    return 0;
}
