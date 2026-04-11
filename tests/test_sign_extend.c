#include <stdio.h>
int main(void) {
    unsigned int a = 0x80000000;
    long long b = (long long)a;
    int result = (b > 0) ? 1 : 0; /* should not sign extend, should be positive */
    printf("%d\n", result);
    return (result == 1) ? 0 : 1;
}
