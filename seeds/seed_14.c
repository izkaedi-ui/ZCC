#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -31;
    int v1 = -30;
    v1 = v1 & v1;
    v1 = ~v1;
    v0 = v0 >> 2;
    v1 = v1 + v1;
    update(v0);
    update(v1);
}
int main(void) {
    f0();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
