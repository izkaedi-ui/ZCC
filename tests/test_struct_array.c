#include <stdio.h>
struct S { int arr[5]; int x; };
int main(void) {
    struct S s;
    s.arr[4] = 99;
    s.x = 42;
    int result = s.arr[4] + s.x;
    printf("%d\n", result);
    return (result == 141) ? 0 : 1;
}
