#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 14;
    int v1 = 17;
    int v2 = 3;
    int v3 = -30;
    int v4 = -25;
    v0 = v3 << 2;
    v0 = ~v2;
    v1 = v2 | v3;
    v3 = (v0 != v1);
    v0 = v3 >> 0;
    v2 = ~v1;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = -40;
    int v1 = -3;
    int v2 = -22;
    int v3 = -47;
    int v4 = -10;
    v1 = v1 >> 0;
    v1 = -v3;
    v4 = -v0;
    v0 = v2 - v1;
    v4 = ~v4;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f2(void) {
    int v0 = -36;
    int v1 = 49;
    int v2 = -14;
    int v3 = -20;
    int v4 = -12;
    v1 = v3 & v4;
    v0 = v3 | v4;
    v4 = v4 ^ v1;
    v2 = (v3 <= v0);
    v1 = v4 << 3;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
int main(void) {
    f0();
    f1();
    f2();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
