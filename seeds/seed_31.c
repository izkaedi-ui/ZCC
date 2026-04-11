#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 42;
    int v1 = 39;
    int v2 = 7;
    int v3 = -10;
    int v4 = -42;
    v0 = v0 ^ v2;
    v1 = v1 - v4;
    v1 = ~v2;
    v3 = ~v3;
    v3 = (v1 >= v1);
    v3 = -v0;
    v1 = ~v3;
    v3 = ~v3;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
int main(void) {
    f0();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
