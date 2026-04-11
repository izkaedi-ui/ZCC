#include <stdio.h>
int main(void) {
    int ch;
    int count;
    count = 0;
    ch = getchar();
    while (ch != (-1)) {
        count = count + 1;
        ch = getchar();
    }
    printf("%d\n", count);
    return 0;
}
