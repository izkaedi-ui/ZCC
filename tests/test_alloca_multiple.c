#include <stdio.h>
int main(void) {
    long A[2];
    long B[2];
    A[1] = 10;
    B[1] = 20;
    int result = A[1] + B[1];
    printf("%d\n", result);
    return (result == 30) ? 0 : 1;
}
