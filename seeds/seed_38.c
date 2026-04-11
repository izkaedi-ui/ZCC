#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -39;
    int v1 = -45;
    int v2 = -21;
    int v3 = 16;
    v3 = v0 << 4;
    v1 = ~v3;
    v0 = v3 - v1;
    v2 = v2 >> 3;
    v3 = v2 << 1;
    v1 = (v0 < v1);
    v3 = (v3 != v2);
    v2 = v1 * v3;
    v2 = -v0;
    v1 = v2 - v0;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
void f1(void) {
    int v0 = -32;
    int v1 = -39;
    int v2 = 41;
    int v3 = -19;
    v0 = -v2;
    v1 = v0 >> 3;
    v1 = ~v0;
    v0 = -v1;
    v0 = v0 - v1;
    v3 = (v2 <= v0);
    v2 = ~v0;
    v0 = v1 - v3;
    v2 = v0 & v0;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
int main(void) {
    f0();
    f1();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
