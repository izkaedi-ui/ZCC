#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 32;
    int v1 = 31;
    int v2 = 29;
    int v3 = -9;
    int v4 = 30;
    int v5 = -10;
    v0 = -v3;
    v5 = -v2;
    v4 = v0 << 2;
    v3 = v3 << 0;
    v2 = v0 << 5;
    v4 = ~v4;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f1(void) {
    int v0 = 9;
    int v1 = 24;
    int v2 = 20;
    int v3 = 44;
    int v4 = -45;
    v1 = ~v2;
    v3 = ~v4;
    v0 = (v3 == v0);
    v4 = v1 - v0;
    v3 = -v4;
    v4 = ~v1;
    v2 = v2 >> 3;
    v4 = v0 >> 5;
    v2 = v0 ^ v4;
    v2 = -v2;
    v1 = ~v2;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f2(void) {
    int v0 = 24;
    int v1 = 34;
    v1 = v1 << 1;
    v1 = v1 << 1;
    v1 = -v0;
    v1 = v0 >> 1;
    v0 = -v0;
    v0 = (v0 < v1);
    update(v0);
    update(v1);
}
void f3(void) {
    int v0 = 48;
    int v1 = 19;
    int v2 = 44;
    int v3 = 17;
    int v4 = -46;
    int v5 = 34;
    v1 = ~v4;
    v1 = -v1;
    v5 = (v4 >= v1);
    v0 = -v3;
    v5 = v5 >> 1;
    v2 = ~v5;
    v1 = -v4;
    v2 = (v3 == v1);
    v3 = ~v3;
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
    f3();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
