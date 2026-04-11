#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 33;
    int v1 = 26;
    int v2 = -33;
    int v3 = -12;
    int v4 = -9;
    v2 = -v1;
    v2 = -v2;
    v4 = -v1;
    v3 = v2 >> 3;
    v4 = v1 | v4;
    v4 = (v0 >= v3);
    v3 = (v2 != v0);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = -33;
    int v1 = 31;
    int v2 = -12;
    int v3 = -20;
    int v4 = -18;
    v3 = -v0;
    v4 = -v3;
    v3 = ~v4;
    v3 = ~v4;
    v2 = v4 ^ v4;
    v0 = v1 - v2;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f2(void) {
    int v0 = -45;
    int v1 = 22;
    int v2 = 32;
    int v3 = 36;
    int v4 = 32;
    int v5 = 1;
    v0 = -v0;
    v2 = v1 ^ v4;
    v4 = ~v4;
    v1 = ~v3;
    v3 = v3 << 5;
    v0 = v3 >> 0;
    v0 = -v0;
    v2 = (v2 != v2);
    v4 = (v3 != v2);
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
