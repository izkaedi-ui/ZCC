#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 26;
    int v1 = -45;
    int v2 = 29;
    int v3 = -40;
    int v4 = 3;
    int v5 = 34;
    v2 = v1 >> 5;
    v2 = (v3 >= v1);
    v3 = v2 << 0;
    v4 = -v4;
    v0 = v4 | v1;
    v1 = v2 << 0;
    v2 = v1 >> 3;
    v5 = ~v4;
    v5 = v4 ^ v2;
    v1 = v2 + v0;
    v1 = ~v2;
    v4 = v1 >> 5;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f1(void) {
    int v0 = 37;
    int v1 = 31;
    int v2 = -17;
    v1 = -v0;
    v2 = v1 + v1;
    v1 = v0 * v2;
    v2 = (v1 >= v2);
    v2 = -v0;
    v0 = v2 | v0;
    v1 = v2 - v2;
    v0 = -v0;
    v1 = v0 << 2;
    v2 = (v0 <= v1);
    v2 = -v2;
    v0 = (v0 != v0);
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
