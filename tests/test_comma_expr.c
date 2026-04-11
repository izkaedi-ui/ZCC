#include <stdio.h>
int main(void) {
    int a = 0, b = 0;
    int result = (a = 2, b = 3, a + b);
    printf("%d\n", result);
    return (result == 5) ? 0 : 1;
}
