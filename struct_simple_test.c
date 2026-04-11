#include <stdio.h>

/* Simple struct with NO padding - all ints */
struct Simple {
    int a;
    int b;
    int c;
};

int main() {
    /* Test brace initializer for simple struct */
    struct Simple s1 = {10, 20, 30};
    printf("s1: a=%d b=%d c=%d\n", s1.a, s1.b, s1.c);
    return 0;
}
