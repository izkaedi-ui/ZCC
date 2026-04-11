#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 3;
    int v1 = 29;
    int v2 = 31;
    int v3 = -47;
    int v4 = 13;
    int v5 = 30;
    v5 = v2 << 3;
    v2 = -v3;
    v5 = v0 & v1;
    v3 = -v3;
    v2 = (v4 == v1);
    v5 = v2 << 3;
    v4 = v4 >> 0;
    v3 = (v0 != v2);
    v1 = (v0 == v0);
    v4 = -v3;
    v1 = (v2 >= v4);
    v1 = v1 >> 3;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
void f1(void) {
    int v0 = -11;
    int v1 = 13;
    int v2 = -48;
    int v3 = -39;
    int v4 = 0;
    v1 = -v1;
    v2 = ~v0;
    v2 = v3 ^ v4;
    v2 = -v4;
    v0 = v3 * v1;
    v3 = v2 >> 0;
    v4 = v4 * v1;
    v2 = ~v0;
    v4 = -v3;
    v2 = ~v4;
    v0 = ~v1;
    v3 = v2 | v2;
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
