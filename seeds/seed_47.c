#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -3;
    int v1 = -7;
    v1 = v1 * v1;
    v1 = -v1;
    v0 = (v0 > v0);
    v1 = v1 - v1;
    v1 = ~v1;
    v0 = ~v0;
    update(v0);
    update(v1);
}
void f1(void) {
    int v0 = 12;
    int v1 = -36;
    int v2 = 2;
    int v3 = 6;
    int v4 = -45;
    v2 = v0 << 5;
    v4 = v4 << 5;
    v1 = -v3;
    v3 = v1 << 4;
    v1 = v4 & v3;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f2(void) {
    int v0 = -41;
    int v1 = 46;
    int v2 = 12;
    v1 = (v1 <= v0);
    v2 = (v2 != v1);
    v0 = (v1 <= v1);
    v0 = -v1;
    v1 = -v0;
    v2 = -v2;
    v0 = v1 + v1;
    v2 = ~v0;
    v2 = v2 * v2;
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
