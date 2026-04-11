#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 2;
    int v1 = -45;
    int v2 = 22;
    int v3 = -22;
    v2 = v3 - v2;
    v2 = (v1 <= v3);
    v3 = (v2 != v2);
    v3 = -v1;
    v1 = v1 >> 5;
    v0 = (v0 >= v0);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
void f1(void) {
    int v0 = 47;
    int v1 = -50;
    v0 = (v0 < v0);
    v1 = (v1 <= v0);
    v1 = ~v1;
    v0 = v1 + v0;
    v1 = ~v1;
    v1 = v0 + v0;
    v0 = ~v0;
    v1 = (v1 != v1);
    v1 = v1 | v1;
    v0 = (v1 >= v0);
    update(v0);
    update(v1);
}
void f2(void) {
    int v0 = -39;
    int v1 = 1;
    v1 = -v0;
    v0 = v0 & v1;
    v0 = v0 << 1;
    v1 = ~v1;
    v0 = v1 - v0;
    v0 = v0 << 4;
    v0 = -v1;
    v0 = (v1 != v1);
    v0 = (v1 < v0);
    v0 = -v0;
    update(v0);
    update(v1);
}
int main(void) {
    f0();
    f1();
    f2();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
