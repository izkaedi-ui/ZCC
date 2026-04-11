#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 23;
    int v1 = 5;
    int v2 = 42;
    v0 = -v1;
    v0 = -v1;
    v0 = (v1 != v1);
    v0 = (v2 <= v2);
    v2 = (v1 != v1);
    v1 = ~v0;
    v1 = v1 & v0;
    v2 = (v2 >= v0);
    v0 = ~v1;
    v1 = v2 >> 3;
    update(v0);
    update(v1);
    update(v2);
}
int main(void) {
    f0();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
