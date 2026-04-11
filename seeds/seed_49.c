#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 47;
    int v1 = -5;
    v1 = -v1;
    v0 = -v0;
    v1 = v0 ^ v0;
    v0 = v1 * v0;
    v0 = v0 >> 5;
    v0 = (v1 < v1);
    v0 = ~v1;
    v1 = (v0 != v1);
    v1 = -v0;
    v1 = v0 - v1;
    v0 = v0 >> 2;
    v0 = (v0 >= v0);
    update(v0);
    update(v1);
}
void f1(void) {
    int v0 = -35;
    int v1 = -2;
    int v2 = -45;
    int v3 = 4;
    int v4 = -15;
    int v5 = 21;
    v1 = (v4 < v2);
    v3 = v5 << 1;
    v2 = (v4 >= v0);
    v5 = ~v2;
    v4 = -v4;
    v0 = v5 << 2;
    v4 = (v0 > v4);
    v4 = -v4;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f2(void) {
    int v0 = -39;
    int v1 = -6;
    int v2 = -22;
    int v3 = -23;
    int v4 = 34;
    v2 = v2 >> 1;
    v0 = ~v0;
    v3 = v1 << 5;
    v2 = v3 & v0;
    v4 = v2 >> 0;
    v4 = ~v3;
    v0 = -v2;
    v2 = (v4 < v0);
    v4 = (v4 >= v0);
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
