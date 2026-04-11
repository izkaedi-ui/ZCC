#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
int main() {
    update(-21);
    update(-14);
    printf("CHECKSUM=%u\n", checksum);
    return 0;
}
