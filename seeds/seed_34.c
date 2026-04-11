#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -43;
    int v1 = -15;
    int v2 = 33;
    int v3 = -34;
    int v4 = 39;
    v1 = (v2 >= v1);
    v3 = -v1;
    v2 = ~v3;
    v3 = -v0;
    v3 = v4 ^ v2;
    v3 = v3 << 2;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = 43;
    int v1 = 8;
    v1 = v0 - v1;
    v1 = -v1;
    v1 = v0 >> 4;
    v0 = v0 >> 0;
    v0 = v0 & v1;
    v1 = v0 ^ v0;
    v0 = v1 >> 0;
    v0 = v1 << 5;
    v1 = v1 & v0;
    update(v0);
    update(v1);
}
int main(void) {
    f0();
    f1();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
