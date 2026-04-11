#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -37;
    int v1 = 5;
    int v2 = -4;
    int v3 = 31;
    int v4 = 8;
    v1 = -v4;
    v4 = v4 >> 3;
    v4 = -v2;
    v1 = v0 >> 2;
    v3 = (v4 >= v4);
    v2 = -v0;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = -9;
    int v1 = -27;
    int v2 = 12;
    int v3 = -23;
    int v4 = -5;
    v2 = v4 >> 5;
    v0 = ~v4;
    v0 = (v1 != v3);
    v1 = ~v3;
    v3 = -v0;
    v2 = v1 ^ v3;
    v2 = (v4 != v2);
    v4 = ~v2;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f2(void) {
    int v0 = 45;
    int v1 = 20;
    int v2 = -8;
    int v3 = -5;
    int v4 = 39;
    v2 = v2 << 1;
    v2 = (v0 >= v4);
    v1 = (v1 == v3);
    v4 = ~v4;
    v0 = v1 << 2;
    v1 = v2 << 0;
    v0 = v0 >> 4;
    v3 = (v0 <= v0);
    v3 = v3 >> 3;
    v0 = (v2 < v3);
    v3 = v3 | v0;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f3(void) {
    int v0 = -31;
    int v1 = -31;
    v0 = v0 | v1;
    v0 = ~v1;
    v1 = -v1;
    v1 = ~v1;
    v0 = ~v0;
    v0 = (v1 > v0);
    v0 = (v0 < v0);
    v1 = -v1;
    update(v0);
    update(v1);
}
int main(void) {
    f0();
    f1();
    f2();
    f3();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
