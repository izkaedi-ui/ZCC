#include <stdio.h>
int main(void) {
    int x = 0;
    goto L1;
L2: x += 20;
    goto end;
L1: x += 10;
    goto L2;
end:
    printf("%d\n", x);
    return (x == 30) ? 0 : 1;
}
