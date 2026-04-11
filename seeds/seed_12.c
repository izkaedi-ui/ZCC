#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 49;
    int v1 = -21;
    int v2 = 27;
    int v3 = -5;
    v2 = (v1 >= v0);
    v2 = v3 | v0;
    v1 = v0 >> 2;
    v1 = -v1;
    v2 = (v2 == v1);
    v2 = -v2;
    v3 = v0 - v1;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
void f1(void) {
    int v0 = 21;
    int v1 = -4;
    int v2 = -39;
    int v3 = 0;
    int v4 = -49;
    v0 = ~v3;
    v2 = v4 >> 3;
    v1 = v3 | v0;
    v2 = ~v4;
    v0 = (v3 >= v2);
    v0 = -v1;
    v0 = v2 + v3;
    v1 = v4 & v1;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f2(void) {
    int v0 = -3;
    int v1 = 35;
    int v2 = 45;
    int v3 = 39;
    int v4 = 19;
    v1 = ~v3;
    v3 = v4 * v3;
    v2 = v1 & v3;
    v2 = (v0 <= v2);
    v0 = v3 << 2;
    v3 = v0 ^ v1;
    v0 = ~v0;
    v1 = v1 << 4;
    v4 = v1 - v4;
    v2 = (v1 < v4);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f3(void) {
    int v0 = -32;
    int v1 = -34;
    int v2 = 19;
    int v3 = -18;
    v0 = v1 * v0;
    v2 = (v0 == v1);
    v1 = v3 ^ v0;
    v3 = v3 | v2;
    v0 = ~v3;
    v1 = ~v0;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
int main(void) {
    f0();
    f1();
    f2();
    f3();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
