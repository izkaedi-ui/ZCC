#include <stdio.h>
int main(void) {
    int A[16];
    int B[16];
    A[15] = 42;
    B[15] = 84;
    int result = A[15] + B[15];
    printf("%d\n", result);
    return (result == 126) ? 0 : 1;
}
