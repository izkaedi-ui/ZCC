#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -46;
    int v1 = 18;
    v0 = v1 << 4;
    v0 = v0 ^ v0;
    v1 = v0 >> 0;
    v1 = ~v0;
    v0 = v1 & v1;
    v1 = ~v1;
    v1 = (v1 > v1);
    update(v0);
    update(v1);
}
void f1(void) {
    int v0 = 1;
    int v1 = -48;
    int v2 = -21;
    v0 = (v2 <= v0);
    v0 = ~v0;
    v2 = v2 >> 0;
    v2 = v2 << 0;
    v2 = v0 ^ v0;
    v1 = v1 + v1;
    v1 = ~v1;
    update(v0);
    update(v1);
    update(v2);
}
void f2(void) {
    int v0 = -24;
    int v1 = 38;
    int v2 = 37;
    v1 = v2 >> 0;
    v2 = ~v2;
    v1 = (v0 >= v2);
    v0 = ~v1;
    update(v0);
    update(v1);
    update(v2);
}
void f3(void) {
    int v0 = 50;
    int v1 = -37;
    v0 = (v1 == v0);
    v1 = -v0;
    v1 = v0 * v1;
    v0 = (v0 < v0);
    v1 = ~v0;
    v0 = v0 ^ v1;
    v1 = (v1 != v1);
    v0 = v0 * v0;
    v1 = v1 << 5;
    v0 = v0 ^ v0;
    v1 = (v1 <= v1);
    update(v0);
    update(v1);
}
int main(void) {
    f0();
    f1();
    f2();
    f3();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
