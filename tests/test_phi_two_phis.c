#include <stdio.h>
int main(void) {
    int a, b;
    int x = 1;
    if (x) { a = 10; b = 20; }
    else { a = 30; b = 40; }
    int result = a + b;
    printf("%d\n", result);
    return (result == 30) ? 0 : 1;
}
