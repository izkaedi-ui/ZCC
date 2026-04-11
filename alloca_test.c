#include <stdio.h>
int main(void) {
    char buf[64];
    int i;
    i = 0;
    while (i < 64) { buf[i] = 'A' + (i % 26); i = i + 1; }
    buf[63] = '\0';
    printf("%s\n", buf);
    return 0;
}
