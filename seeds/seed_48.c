#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -7;
    int v1 = -4;
    int v2 = -44;
    int v3 = -22;
    int v4 = 8;
    v4 = ~v4;
    v2 = -v3;
    v0 = v1 * v0;
    v2 = v4 >> 4;
    v0 = v1 - v3;
    v0 = -v2;
    v1 = ~v1;
    v3 = v1 - v3;
    v0 = v3 << 0;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = 8;
    int v1 = 34;
    int v2 = 25;
    int v3 = 14;
    int v4 = -33;
    v4 = v0 & v4;
    v3 = ~v4;
    v4 = (v1 < v1);
    v4 = -v4;
    v4 = v3 << 3;
    v3 = v2 >> 2;
    v1 = (v3 >= v4);
    v2 = ~v4;
    v4 = v1 << 1;
    v2 = ~v2;
    v1 = (v4 >= v4);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
int main(void) {
    f0();
    f1();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
