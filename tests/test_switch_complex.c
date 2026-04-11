#include <stdio.h>
int main(void) {
    int x = 2;
    int result = 0;
    switch (x) {
        case 1: result = 10; break;
        case 2: result = 20; /* fallthrough */
        case 3: result += 30; break;
        default: result = 100;
    }
    printf("%d\n", result);
    return (result == 50) ? 0 : 1;
}
