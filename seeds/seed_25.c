#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 16;
    int v1 = 6;
    int v2 = -36;
    int v3 = 37;
    v3 = ~v0;
    v3 = ~v0;
    v1 = -v3;
    v0 = -v3;
    v0 = v3 << 2;
    v0 = (v0 == v3);
    v0 = v0 + v3;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
void f1(void) {
    int v0 = 2;
    int v1 = -27;
    int v2 = 48;
    int v3 = -33;
    v3 = v3 >> 3;
    v1 = v2 - v0;
    v3 = ~v1;
    v0 = (v0 != v2);
    v3 = ~v3;
    v1 = (v3 > v2);
    v1 = v0 >> 0;
    v0 = ~v1;
    v0 = (v0 != v0);
    v0 = ~v1;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
int main(void) {
    f0();
    f1();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
