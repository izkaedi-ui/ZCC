#include <stdio.h>
int h(int x) { return x + 1; }
int g(int x) { return h(x) * 2; }
int f(int x) { return g(x) - 3; }
int main(void) {
    int result = f(4);
    printf("%d\n", result);
    return (result == 7) ? 0 : 1;
}
