#include <stdio.h>

int main(void) {
    double d = 300.7;
    float  f = 300.7f;

    short          ds  = d;    printf("double->short:  %d\n", (int)ds);
    unsigned short dus = d;    printf("double->ushort: %u\n", (unsigned)dus);
    signed char    dc  = d;    printf("double->char:   %d\n", (int)dc);
    unsigned char  duc = d;    printf("double->uchar:  %u\n", (unsigned)duc);

    short          fs  = f;    printf("float->short:   %d\n", (int)fs);
    unsigned short fus = f;    printf("float->ushort:  %u\n", (unsigned)fus);
    signed char    fc  = f;    printf("float->char:    %d\n", (int)fc);
    unsigned char  fuc = f;    printf("float->uchar:   %u\n", (unsigned)fuc);

    /* Negative-value sibling probe — tests sign-bit pathway: */
    double dn = -300.7;
    short  dns = dn;   printf("neg double->short: %d\n", (int)dns);
    signed char dnc = dn; printf("neg double->char:  %d\n", (int)dnc);

    return 0;
}
