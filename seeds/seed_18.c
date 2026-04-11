#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -4;
    int v1 = -42;
    int v2 = 17;
    int v3 = 19;
    int v4 = 14;
    int v5 = 50;
    v0 = ~v3;
    v0 = -v5;
    v2 = -v2;
    v2 = v0 - v2;
    v4 = v5 - v2;
    v2 = v4 ^ v2;
    v5 = (v3 != v5);
    v1 = (v0 != v5);
    v2 = v1 - v0;
    v2 = v2 & v4;
    v3 = ~v2;
    v5 = v1 * v2;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
int main(void) {
    f0();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
