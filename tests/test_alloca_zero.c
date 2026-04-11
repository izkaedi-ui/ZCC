#include <stdio.h>
int main(void) {
    int A[1]; /* Zero sized arrays are illegal in strict C89, using 1 to avoid parser panic */
    int result = 0;
    printf("%d\n", result);
    return result;
}
