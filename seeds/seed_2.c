#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 27;
    int v1 = -25;
    int v2 = -31;
    int v3 = -3;
    int v4 = 47;
    int v5 = -30;
    v0 = ~v4;
    v3 = v0 >> 0;
    v1 = v0 << 1;
    v5 = v3 | v0;
    v1 = (v5 <= v3);
    v2 = (v4 != v4);
    v4 = (v5 > v5);
    v3 = v5 >> 5;
    v4 = -v3;
    v1 = v1 * v0;
    v4 = v4 | v1;
    v0 = (v0 >= v5);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f1(void) {
    int v0 = -21;
    int v1 = -42;
    v0 = v0 >> 2;
    v0 = (v1 != v0);
    v0 = -v0;
    v1 = v1 & v1;
    update(v0);
    update(v1);
}
void f2(void) {
    int v0 = 43;
    int v1 = -44;
    int v2 = 36;
    int v3 = 33;
    int v4 = 32;
    v3 = v2 - v0;
    v1 = (v4 > v3);
    v1 = -v2;
    v1 = -v0;
    v4 = -v0;
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
