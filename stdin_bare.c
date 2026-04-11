#include <stdio.h>
int main(void) {
    FILE *f;
    int ch;
    f = stdin;
    ch = fgetc(f);
    printf("%d\n", ch);
    return 0;
}
