#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -13;
    int v1 = 32;
    int v2 = -44;
    int v3 = 50;
    int v4 = -20;
    v0 = -v1;
    v4 = -v0;
    v0 = ~v2;
    v1 = v3 >> 4;
    v4 = v1 << 3;
    v4 = v0 * v0;
    v0 = (v0 <= v3);
    v4 = v3 >> 3;
    v4 = v4 - v4;
    v0 = v2 << 0;
    v4 = (v2 <= v3);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = -11;
    int v1 = 39;
    v0 = (v0 != v0);
    v1 = v0 ^ v1;
    v0 = (v0 >= v0);
    v0 = v1 | v1;
    v1 = ~v0;
    v0 = v1 << 5;
    v1 = v1 | v1;
    v1 = v0 & v1;
    v1 = ~v0;
    v0 = -v1;
    update(v0);
    update(v1);
}
int main(void) {
    f0();
    f1();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
