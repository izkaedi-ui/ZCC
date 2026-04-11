#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 32;
    int v1 = 19;
    v1 = (v1 > v1);
    v0 = -v1;
    v1 = -v1;
    v1 = (v0 >= v0);
    v0 = v1 | v1;
    v0 = (v1 <= v1);
    v0 = -v1;
    v0 = v1 << 5;
    v0 = ~v0;
    v0 = -v0;
    update(v0);
    update(v1);
}
void f1(void) {
    int v0 = -26;
    int v1 = -34;
    int v2 = -45;
    v2 = v1 ^ v0;
    v2 = (v1 >= v1);
    v0 = v1 & v2;
    v1 = ~v1;
    v1 = ~v0;
    v2 = ~v2;
    v0 = -v1;
    v0 = (v2 <= v2);
    v0 = v2 * v0;
    v1 = ~v1;
    update(v0);
    update(v1);
    update(v2);
}
void f2(void) {
    int v0 = -1;
    int v1 = 33;
    int v2 = 12;
    int v3 = 23;
    v1 = v2 - v3;
    v3 = ~v0;
    v2 = -v1;
    v3 = v2 + v2;
    v2 = v2 >> 2;
    v1 = v1 ^ v2;
    v2 = -v2;
    v0 = -v1;
    v1 = -v3;
    v2 = -v0;
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
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
