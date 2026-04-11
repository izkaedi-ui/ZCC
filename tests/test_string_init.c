#include <stdio.h>
int main(void) {
    char *s = "hello";
    int result = s[1]; /* 'e' == 101 */
    printf("%d\n", result);
    return (result == 101) ? 0 : 1;
}
