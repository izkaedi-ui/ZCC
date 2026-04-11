#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -34;
    int v1 = -28;
    int v2 = 43;
    int v3 = -50;
    v0 = ~v1;
    v2 = ~v1;
    v0 = ~v3;
    v2 = (v0 == v1);
    v1 = v0 >> 5;
    v0 = -v2;
    v1 = (v0 == v2);
    v0 = v0 & v3;
    v1 = -v3;
    v3 = -v2;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
void f1(void) {
    int v0 = -2;
    int v1 = -38;
    int v2 = 11;
    int v3 = 50;
    int v4 = 23;
    int v5 = 33;
    v3 = (v0 == v0);
    v2 = v2 + v0;
    v1 = v3 << 1;
    v0 = ~v2;
    v4 = ~v3;
    v3 = v5 | v1;
    v1 = -v0;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f2(void) {
    int v0 = -8;
    int v1 = -24;
    v1 = -v1;
    v0 = v0 >> 0;
    v1 = ~v0;
    v1 = v0 + v1;
    v1 = v0 << 0;
    v0 = (v1 != v0);
    v1 = v1 & v1;
    update(v0);
    update(v1);
}
int main(void) {
    f0();
    f1();
    f2();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
