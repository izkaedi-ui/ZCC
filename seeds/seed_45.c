#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 33;
    int v1 = -39;
    int v2 = -22;
    int v3 = -5;
    v3 = v2 >> 2;
    v2 = ~v3;
    v3 = v0 & v0;
    v0 = (v3 != v2);
    v1 = v1 * v1;
    v3 = -v1;
    v1 = v0 >> 3;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
void f1(void) {
    int v0 = -10;
    int v1 = 31;
    int v2 = -43;
    v1 = -v0;
    v2 = (v0 < v1);
    v2 = (v1 != v2);
    v1 = -v1;
    update(v0);
    update(v1);
    update(v2);
}
int main(void) {
    f0();
    f1();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
