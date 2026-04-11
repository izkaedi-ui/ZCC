#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { printf("up %d\n", v); }
void f0(void) {
    int v0 = -21;
    int v1 = -14;
    v0 = -v0;
    v0 = v1 << 0;
    v1 = (v0 < v0);
    v1 = (v1 < v1);
    v1 = -v1;
    v1 = v1 << 0;
    v1 = -v1;
    v0 = v0 ^ v0;
    update(v0);
    update(v1);
}
void f1(void) {
    int v0 = 23;
    int v1 = -45;
    int v2 = 47;
    int v3 = 46;
    v3 = -v2;
    v3 = (v3 != v0);
    v3 = v2 << 2;
    v2 = (v3 == v3);
    v0 = -v3;
    v2 = v2 + v2;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
void f2(void) {
    int v0 = 15;
    int v1 = -50;
    int v2 = 34;
    int v3 = 19;
    int v4 = 9;
    v1 = v4 | v2;
    v3 = -v0;
    v2 = (v4 == v1);
    v3 = -v0;
    v4 = v1 * v1;
    v0 = ~v4;
    v0 = -v1;
    v3 = v0 & v1;
    v4 = v2 >> 3;
    v1 = -v3;
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
