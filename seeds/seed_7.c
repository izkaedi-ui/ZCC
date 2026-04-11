#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 28;
    int v1 = 42;
    int v2 = 14;
    int v3 = 4;
    int v4 = 20;
    v3 = (v3 > v2);
    v4 = v3 << 5;
    v3 = v0 >> 5;
    v2 = (v2 <= v2);
    v1 = v1 & v1;
    v1 = (v0 != v3);
    v4 = v3 << 3;
    v3 = (v3 >= v4);
    v4 = v3 + v3;
    v2 = v3 << 3;
    v1 = -v2;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = 12;
    int v1 = -47;
    int v2 = -1;
    int v3 = -7;
    int v4 = 35;
    v3 = (v1 <= v4);
    v3 = v4 ^ v4;
    v0 = v3 & v1;
    v0 = (v2 == v3);
    v3 = (v2 != v2);
    v3 = v2 >> 0;
    v4 = v0 - v2;
    v0 = v0 - v1;
    v4 = v1 - v1;
    v0 = -v4;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f2(void) {
    int v0 = 9;
    int v1 = 39;
    int v2 = -18;
    v2 = (v2 >= v2);
    v0 = v1 | v0;
    v1 = v2 & v2;
    v2 = -v0;
    v2 = v2 - v2;
    v2 = v1 | v2;
    v2 = v0 | v1;
    v2 = -v1;
    v2 = v2 + v1;
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
