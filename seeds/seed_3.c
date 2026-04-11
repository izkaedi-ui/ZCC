#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -49;
    int v1 = -39;
    int v2 = 46;
    int v3 = -20;
    int v4 = -29;
    int v5 = 2;
    v1 = -v3;
    v1 = v3 & v0;
    v3 = v2 >> 3;
    v1 = (v2 < v1);
    v5 = ~v4;
    v5 = v2 + v0;
    v3 = ~v4;
    v1 = ~v0;
    v0 = ~v1;
    v4 = v0 - v5;
    v0 = -v4;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
    update(v5);
}
int main(void) {
    f0();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
