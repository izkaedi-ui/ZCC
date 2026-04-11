#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -2;
    int v1 = -10;
    int v2 = -28;
    int v3 = 8;
    int v4 = 18;
    int v5 = -7;
    v5 = v5 >> 5;
    v3 = ~v1;
    v2 = (v4 > v2);
    v2 = v5 >> 1;
    v3 = v2 >> 4;
    v0 = v4 >> 5;
    v2 = -v1;
    v0 = v2 << 5;
    v3 = v5 >> 2;
    v1 = (v4 <= v2);
    v1 = v0 << 4;
    v0 = (v5 > v4);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f1(void) {
    int v0 = -44;
    int v1 = -38;
    int v2 = 2;
    v0 = -v2;
    v0 = (v2 != v0);
    v1 = -v2;
    v1 = (v1 >= v1);
    v0 = v2 - v2;
    v2 = ~v2;
    v0 = v1 + v0;
    v1 = -v0;
    v0 = v0 << 2;
    update(v0);
    update(v1);
    update(v2);
}
void f2(void) {
    int v0 = -14;
    int v1 = 8;
    int v2 = 32;
    int v3 = 19;
    v1 = -v3;
    v1 = v0 << 2;
    v2 = -v1;
    v2 = v2 - v1;
    v3 = ~v3;
    v2 = ~v0;
    v0 = -v1;
    v3 = (v2 < v1);
    v3 = v1 >> 3;
    v2 = v0 >> 2;
    v0 = (v0 == v3);
    v3 = (v2 >= v0);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
void f3(void) {
    int v0 = -3;
    int v1 = 28;
    int v2 = -22;
    int v3 = -22;
    v1 = -v3;
    v2 = ~v3;
    v3 = ~v1;
    v0 = (v3 < v2);
    v0 = ~v0;
    v1 = ~v1;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
int main(void) {
    f0();
    f1();
    f2();
    f3();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
