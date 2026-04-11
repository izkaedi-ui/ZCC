#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -21;
    int v1 = 45;
    int v2 = -39;
    int v3 = 5;
    v3 = v1 + v2;
    v2 = v0 * v2;
    v3 = v1 >> 1;
    v1 = ~v1;
    v0 = (v3 != v1);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
void f1(void) {
    int v0 = -32;
    int v1 = -21;
    int v2 = 9;
    int v3 = 31;
    int v4 = -18;
    int v5 = 8;
    v3 = v2 | v5;
    v0 = (v3 <= v2);
    v5 = v3 >> 5;
    v2 = -v1;
    v3 = -v0;
    v3 = (v4 <= v2);
    v5 = v2 >> 0;
    v0 = v4 << 5;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
int main(void) {
    f0();
    f1();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
