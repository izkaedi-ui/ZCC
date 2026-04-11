#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -16;
    int v1 = -27;
    int v2 = -47;
    int v3 = -10;
    int v4 = 26;
    v2 = v0 | v3;
    v4 = v1 << 1;
    v1 = (v2 >= v4);
    v1 = v4 * v3;
    v1 = (v1 == v4);
    v4 = v0 >> 5;
    v4 = ~v1;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = -26;
    int v1 = 13;
    int v2 = 19;
    int v3 = -11;
    int v4 = -28;
    int v5 = 12;
    v0 = v1 - v4;
    v4 = v3 * v0;
    v1 = ~v1;
    v5 = v1 << 2;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f2(void) {
    int v0 = -32;
    int v1 = 25;
    v1 = v0 ^ v1;
    v0 = -v1;
    v1 = v0 >> 1;
    v1 = -v0;
    v1 = ~v0;
    v0 = ~v1;
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
