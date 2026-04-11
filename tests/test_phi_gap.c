#include <stdio.h>
int main(void) {
    int a, b;
    int x = 1;
    if (x) { a = 10; b = 20; }
    else { a = 30; b = 40; }
    /* The loop jump creates PHI nodes merging back */
    while (x < 3) {
        a++;
        b++;
        x++;
    }
    int result = a + b;
    printf("%d\n", result);
    return (result == 34) ? 0 : 1;
}
