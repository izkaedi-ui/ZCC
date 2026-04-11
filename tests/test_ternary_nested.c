#include <stdio.h>
int main(void) {
    int a=1, b=0, c=2, d=3;
    int result = a ? (b ? c : d) : 0;
    printf("%d\n", result);
    return (result == 3) ? 0 : 1;
}
