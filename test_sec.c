// Quick test: write to a file that security-416 is working
// Run: ZCC_EMIT_IR=1 ./zcc.exe --security-416 test_sec.c
#include <stdlib.h>
void test(void) {
    char *p = malloc(10);
    free(p);
    p[0] = 'A';  // UAF
}
int main(void) { test(); return 0; }
