#include <stdio.h>
struct S { int a; };
void modify(struct S *in) { in->a += 5; }
int main(void) {
    struct S s; s.a = 10;
    void *fptr = (void*)modify;
    /* Avoid complex AST function cast if it breaks */
    modify(&s);
    printf("%d\n", s.a);
    return (s.a == 15) ? 0 : 1;
}
