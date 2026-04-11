#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 29;
    int v1 = -28;
    int v2 = 36;
    int v3 = 40;
    int v4 = -12;
    v2 = (v0 == v1);
    v1 = v2 | v1;
    v3 = -v4;
    v4 = (v3 > v3);
    v4 = -v1;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = -29;
    int v1 = 12;
    int v2 = -14;
    int v3 = -33;
    int v4 = -27;
    int v5 = -10;
    v0 = ~v2;
    v3 = v1 & v1;
    v4 = ~v5;
    v3 = -v5;
    v3 = -v5;
    v3 = -v1;
    v4 = v0 * v1;
    v2 = v1 * v4;
    v3 = (v4 != v5);
    v4 = ~v0;
    v0 = ~v2;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f2(void) {
    int v0 = -4;
    int v1 = 19;
    int v2 = 46;
    int v3 = -45;
    int v4 = 47;
    int v5 = 42;
    v1 = ~v3;
    v5 = v5 * v1;
    v3 = v2 + v2;
    v0 = -v1;
    v5 = (v4 == v5);
    v5 = ~v3;
    v4 = (v1 > v1);
    v4 = v2 * v4;
    v4 = ~v1;
    v5 = v2 >> 2;
    v5 = ~v0;
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
