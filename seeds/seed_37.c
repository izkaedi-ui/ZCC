#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 48;
    int v1 = -37;
    int v2 = 23;
    int v3 = 36;
    v0 = (v0 > v0);
    v3 = v2 - v3;
    v0 = (v3 == v1);
    v3 = (v2 < v2);
    v2 = ~v0;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
void f1(void) {
    int v0 = 36;
    int v1 = -44;
    int v2 = -28;
    int v3 = -22;
    int v4 = 16;
    int v5 = -45;
    v3 = v2 * v2;
    v4 = v3 * v0;
    v2 = (v4 >= v2);
    v1 = ~v3;
    v4 = v3 << 1;
    v0 = (v3 > v0);
    v2 = ~v5;
    v2 = v0 & v5;
    v0 = v1 << 4;
    v3 = -v0;
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
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
