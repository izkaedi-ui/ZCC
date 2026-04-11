#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -20;
    int v1 = 49;
    v1 = v0 << 4;
    v1 = -v1;
    v0 = -v0;
    v1 = ~v1;
    v1 = v0 << 0;
    v1 = (v1 == v0);
    v1 = v0 >> 4;
    v0 = -v0;
    v0 = (v1 >= v0);
    update(v0);
    update(v1);
}
void f1(void) {
    int v0 = -39;
    int v1 = 37;
    int v2 = -10;
    int v3 = 35;
    int v4 = -6;
    v4 = ~v2;
    v1 = v1 << 2;
    v1 = v1 & v1;
    v2 = v4 * v4;
    v4 = -v1;
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
