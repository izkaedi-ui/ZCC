#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 27;
    int v1 = 15;
    int v2 = -36;
    int v3 = -1;
    int v4 = 23;
    v0 = v3 << 0;
    v3 = v0 >> 5;
    v2 = ~v0;
    v4 = v2 >> 5;
    v3 = v2 << 4;
    v3 = (v3 <= v1);
    v2 = ~v3;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = -50;
    int v1 = -12;
    int v2 = -14;
    int v3 = -24;
    int v4 = 5;
    int v5 = 50;
    v3 = -v3;
    v4 = (v3 > v5);
    v2 = v4 ^ v5;
    v2 = ~v0;
    v5 = (v2 > v1);
    v0 = (v0 != v1);
    v0 = ~v3;
    v5 = -v4;
    v5 = (v5 != v3);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f2(void) {
    int v0 = -19;
    int v1 = -32;
    int v2 = 33;
    int v3 = 38;
    int v4 = -50;
    v1 = -v1;
    v3 = ~v0;
    v1 = ~v0;
    v1 = -v3;
    v4 = ~v4;
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
