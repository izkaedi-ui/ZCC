#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 1;
    int v1 = -33;
    int v2 = 49;
    int v3 = 17;
    int v4 = 22;
    int v5 = -47;
    v1 = (v0 < v1);
    v0 = -v2;
    v2 = (v2 <= v3);
    v5 = -v0;
    v1 = v2 | v3;
    v1 = -v3;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f1(void) {
    int v0 = 20;
    int v1 = -6;
    int v2 = -6;
    int v3 = -30;
    int v4 = -17;
    int v5 = 45;
    v0 = v3 << 0;
    v1 = ~v1;
    v5 = (v1 >= v2);
    v4 = -v0;
    v3 = v1 - v5;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f2(void) {
    int v0 = -49;
    int v1 = -22;
    int v2 = -18;
    int v3 = 27;
    int v4 = -11;
    int v5 = 41;
    v3 = -v2;
    v2 = -v1;
    v2 = -v5;
    v4 = ~v3;
    v0 = ~v0;
    v5 = ~v2;
    v5 = ~v4;
    v2 = ~v2;
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
