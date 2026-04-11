#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -16;
    int v1 = -35;
    int v2 = -3;
    int v3 = 5;
    v3 = -v2;
    v3 = (v3 <= v2);
    v0 = v3 >> 0;
    v1 = ~v2;
    v0 = v0 >> 2;
    v3 = v3 >> 1;
    v3 = v0 >> 5;
    v3 = ~v2;
    v0 = v3 | v2;
    v0 = (v1 < v3);
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
int main(void) {
    f0();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
