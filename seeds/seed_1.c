#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -17;
    int v1 = -45;
    int v2 = 43;
    int v3 = 8;
    int v4 = 18;
    int v5 = -35;
    v4 = v2 | v5;
    v4 = v1 << 5;
    v5 = v1 + v2;
    v0 = (v3 != v2);
    v1 = v2 << 2;
    v5 = v5 << 5;
    v5 = ~v1;
    v5 = ~v1;
    v3 = (v3 >= v2);
    v1 = ~v5;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f1(void) {
    int v0 = 48;
    int v1 = 49;
    int v2 = -43;
    int v3 = -21;
    v3 = v2 << 0;
    v2 = ~v1;
    v3 = -v3;
    v2 = (v1 >= v1);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
void f2(void) {
    int v0 = 18;
    int v1 = -17;
    int v2 = 45;
    int v3 = 24;
    int v4 = 4;
    int v5 = 24;
    v1 = v1 >> 4;
    v0 = v0 ^ v1;
    v5 = (v3 < v4);
    v3 = -v4;
    v4 = -v2;
    v0 = ~v5;
    v5 = v4 ^ v2;
    v0 = v2 << 3;
    v0 = -v5;
    v4 = v1 << 4;
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
