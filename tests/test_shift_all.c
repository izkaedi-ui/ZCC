#include <stdio.h>
int main(void) {
    int a = 1 << 4; /* 16 */
    unsigned int b = 0xF0000000 >> 4; /* 0x0F000000 */
    int c = -16 >> 2; /* -4 */
    int result = (a == 16 && b == 0x0F000000 && c == -4) ? 100 : 0;
    printf("%d\n", result);
    return (result == 100) ? 0 : 1;
}
