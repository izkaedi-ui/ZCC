#include <stdio.h>
const float arr[4] = {1.5f, 2.5f, 3.5f, 4.5f};
int main(void) {
    float x = arr[2];
    printf("arr[2] = %f (expected: 3.5)\n", x);
    if (x < 3.49f || x > 3.51f) return 1;
    return 0;
}
