#include <stdio.h>
int main(void) {
    int a[3];
    int b[3];
    a[0] = 0; a[1] = 0; a[2] = 0;
    b[0] = 10; b[1] = 20; b[2] = 30;
    int i = 0, j = 1;
    a[i++] = b[j++];
    int result = a[0] + i + j; /* 20 + 1 + 2 = 23 */
    printf("%d\n", result);
    return (result == 23) ? 0 : 1;
}
