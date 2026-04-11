#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 48;
    int v1 = 28;
    int v2 = 26;
    int v3 = 0;
    int v4 = 49;
    int v5 = -9;
    v3 = v3 >> 1;
    v3 = ~v3;
    v5 = (v4 != v5);
    v4 = v2 ^ v1;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f1(void) {
    int v0 = -40;
    int v1 = -16;
    v0 = -v1;
    v1 = v0 << 0;
    v1 = v1 << 2;
    v0 = (v0 != v1);
    v1 = ~v0;
    v0 = v1 | v0;
    update(v0);
    update(v1);
}
void f2(void) {
    int v0 = 50;
    int v1 = -9;
    int v2 = -1;
    int v3 = -49;
    int v4 = -13;
    int v5 = 2;
    v5 = v4 | v1;
    v1 = ~v5;
    v1 = -v1;
    v2 = v2 << 3;
    v1 = v3 & v2;
    v3 = v0 >> 2;
    v5 = (v5 <= v3);
    v1 = ~v3;
    v1 = v2 & v0;
    v4 = v3 >> 2;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
int main(void) {
    f0();
    f1();
    f2();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
