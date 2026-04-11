#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -24;
    int v1 = -15;
    int v2 = 46;
    v0 = ~v2;
    v1 = (v1 != v0);
    v1 = v2 | v1;
    v2 = v1 >> 4;
    v0 = ~v1;
    update(v0);
    update(v1);
    update(v2);
}
int main(void) {
    f0();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
