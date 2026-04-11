#include <stdio.h>
int main() {
    int v0 = -21;
    int v1 = -14;
    v0 = -v0;
    printf("1: %d\n", v0);
    v0 = v1 << 0;
    printf("2: %d\n", v0);
    v1 = (v0 < v0);
    printf("3: %d\n", v1);
    v1 = (v1 < v1);
    printf("4: %d\n", v1);
    v1 = -v1;
    printf("5: %d\n", v1);
    v1 = v1 << 0;
    printf("6: %d\n", v1);
    v1 = -v1;
    printf("7: %d\n", v1);
    v0 = v0 ^ v0;
    printf("8: %d\n", v0);
    return 0;
}
