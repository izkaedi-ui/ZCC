#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = 44;
    int v1 = -15;
    v0 = (v0 <= v0);
    v0 = -v0;
    v0 = v0 | v0;
    v1 = (v0 <= v1);
    v0 = v0 >> 5;
    v1 = v0 >> 1;
    v0 = v1 * v0;
    update(v0);
    update(v1);
}
int main(void) {
    f0();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
