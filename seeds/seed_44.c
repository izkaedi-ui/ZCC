#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -42;
    int v1 = -7;
    int v2 = -15;
    int v3 = 31;
    int v4 = -9;
    v2 = v1 << 1;
    v4 = v3 ^ v2;
    v0 = (v3 == v2);
    v3 = -v3;
    v4 = v4 * v1;
    v4 = (v3 <= v2);
    v1 = v4 >> 1;
    v3 = v1 ^ v2;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
int main(void) {
    f0();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
