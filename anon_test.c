#include <stdio.h>

struct Outer {
    int before;
    union {
        int a;
        long b;
    };
    int after;
};

int main(void) {
    struct Outer x;
    x.before = 1;
    x.a = 42;
    x.after = 3;
    printf("%d %d %d\n", x.before, x.a, x.after);
    return 0;
}
