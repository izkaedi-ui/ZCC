#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -43;
    int v1 = 21;
    int v2 = 25;
    int v3 = 44;
    v3 = -v0;
    v3 = v2 >> 5;
    v1 = -v2;
    v1 = -v3;
    v1 = (v2 != v1);
    v3 = v3 & v1;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
void f1(void) {
    int v0 = -18;
    int v1 = -24;
    int v2 = -8;
    v1 = -v0;
    v2 = ~v0;
    v1 = v1 | v2;
    v0 = ~v0;
    v2 = (v2 <= v2);
    update(v0);
    update(v1);
    update(v2);
}
void f2(void) {
    int v0 = 11;
    int v1 = -24;
    int v2 = -8;
    v0 = v0 + v2;
    v0 = -v2;
    v2 = ~v0;
    v0 = -v2;
    v1 = v1 >> 3;
    v1 = v1 >> 4;
    v0 = -v2;
    v1 = -v0;
    update(v0);
    update(v1);
    update(v2);
}
int main(void) {
    f0();
    f1();
    f2();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
