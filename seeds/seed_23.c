#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -25;
    int v1 = 27;
    int v2 = 15;
    v0 = ~v0;
    v0 = v2 & v1;
    v1 = ~v0;
    v2 = ~v0;
    v2 = v1 >> 1;
    v2 = ~v1;
    v1 = ~v2;
    v2 = ~v1;
    v1 = ~v1;
    v1 = v1 + v2;
    update(v0);
    update(v1);
    update(v2);
}
void f1(void) {
    int v0 = 24;
    int v1 = -11;
    int v2 = 42;
    int v3 = 38;
    int v4 = 30;
    v3 = ~v2;
    v4 = ~v1;
    v4 = v3 | v1;
    v3 = ~v1;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f2(void) {
    int v0 = -46;
    int v1 = 23;
    int v2 = 39;
    v0 = (v1 != v1);
    v1 = (v2 != v0);
    v2 = ~v1;
    v2 = v0 - v2;
    v1 = ~v2;
    update(v0);
    update(v1);
    update(v2);
}
int main(void) {
    f0();
    f1();
    f2();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
