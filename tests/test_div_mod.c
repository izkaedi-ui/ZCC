#include <stdio.h>
int main(void) {
    int a = -10;
    int b = 3;
    unsigned int c = 100;
    unsigned int d = 15;
    int r1 = a / b;
    int r2 = a % b;
    unsigned int r3 = c / d;
    unsigned int r4 = c % d;
    int result = r1 + r2 + (int)r3 + (int)r4; /* -3 + -1 + 6 + 10 = 12 */
    printf("%d\n", result);
    return (result == 12) ? 0 : 1;
}
