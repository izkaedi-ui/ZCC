#include <stdio.h>
#include <string.h>
int main(int argc, char **argv) {
    int ch;
    int count;
    FILE *f;
    count = 0;
    if (argc > 1) {
        f = fopen(argv[1], "r");
        if (!f) return 1;
    } else {
        f = stdin;
    }
    ch = fgetc(f);
    while (ch != (-1)) {
        count = count + 1;
        ch = fgetc(f);
    }
    if (f != stdin) fclose(f);
    printf("%d\n", count);
    return 0;
}
