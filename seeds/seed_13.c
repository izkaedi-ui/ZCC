#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 32;
    int v1 = -47;
    int v2 = -43;
    int v3 = 11;
    int v4 = 1;
    v3 = v3 + v0;
    v4 = v1 << 0;
    v4 = v4 >> 4;
    v4 = -v4;
    v3 = v4 >> 4;
    v0 = v4 & v1;
    v1 = -v3;
    v3 = v3 << 3;
    v3 = v2 >> 5;
    v1 = v3 << 0;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = -39;
    int v1 = 5;
    v0 = v0 >> 4;
    v1 = v1 ^ v1;
    v1 = v1 + v1;
    v0 = ~v0;
    v0 = -v0;
    update(v0);
    update(v1);
}
void f2(void) {
    int v0 = 21;
    int v1 = -3;
    int v2 = -36;
    int v3 = 47;
    v1 = ~v3;
    v0 = ~v2;
    v1 = v2 * v2;
    v0 = v1 >> 1;
    v1 = v0 ^ v0;
    v1 = ~v3;
    v3 = -v2;
    v2 = (v2 <= v2);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
int main(void) {
    f0();
    f1();
    f2();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
