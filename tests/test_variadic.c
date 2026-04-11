#include <stdio.h>
int sum_7(int a, int b, int c, int d, int e, int f, int g) {
    return a+b+c+d+e+f+g;
}
int main(void) {
    int result = sum_7(1, 2, 3, 4, 5, 6, 7);
    printf("%d\n", result);
    return (result == 28) ? 0 : 1;
}
