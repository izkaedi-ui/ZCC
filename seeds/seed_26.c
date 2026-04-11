#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 6;
    int v1 = -21;
    int v2 = 19;
    int v3 = -23;
    int v4 = 46;
    v4 = (v2 >= v1);
    v2 = ~v4;
    v2 = ~v1;
    v1 = v4 >> 1;
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
