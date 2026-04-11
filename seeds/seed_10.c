#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
void f0(void) {
    int v0 = -26;
    int v1 = 26;
    int v2 = 15;
    int v3 = 45;
    int v4 = -33;
    v3 = v2 >> 4;
    v2 = v2 | v4;
    v1 = -v3;
    v3 = ~v2;
    v4 = v4 >> 3;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
    update(v4);
}
void f1(void) {
    int v0 = -26;
    int v1 = 39;
    int v2 = -20;
    int v3 = 23;
    v3 = (v0 >= v2);
    v3 = -v3;
    v3 = (v0 <= v1);
    v2 = ~v0;
    v0 = -v3;
    v1 = v3 + v1;
    v2 = -v2;
    v3 = ~v0;
    v3 = v2 >> 5;
    v0 = ~v0;
    update(v0);
    update(v1);
    update(v2);
    update(v3);
}
int main(void) {
    f0();
    f1();
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
