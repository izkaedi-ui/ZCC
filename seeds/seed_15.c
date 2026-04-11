#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 26;
    int v1 = -43;
    int v2 = -50;
    int v3 = -24;
    int v4 = -12;
    v2 = (v2 < v2);
    v3 = v3 - v1;
    v4 = -v1;
    v4 = ~v2;
    v3 = v0 + v3;
    v0 = -v2;
    v3 = ~v4;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = 40;
    int v1 = 31;
    int v2 = 3;
    int v3 = -13;
    int v4 = -36;
    v2 = v1 & v4;
    v0 = v3 << 0;
    v4 = -v3;
    v0 = ~v3;
    v2 = v1 << 2;
    v4 = v0 | v4;
    v2 = (v2 > v1);
    v1 = v2 - v1;
    v1 = ~v0;
    v3 = (v3 <= v4);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
int main(void) {
    f0();
    f1();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
