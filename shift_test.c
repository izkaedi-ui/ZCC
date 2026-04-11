#include <stdio.h>
int v1;
void update(int x) { v1 = v1 + x; }
int main(void) {
    v1 = 42;
    v1 = v1 << 0;
    update(v1);
    printf("CHECKSUM=%u\n", (unsigned)v1);
    return 0;
}
