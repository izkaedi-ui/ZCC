#include <stdio.h>
#include <part1.c>
int main() {
    FILE *f = fopen("test3_out.txt", "w");
    fprintf(f, "ND_RETURN=%d, ND_BLOCK=%d, ND_CASE=%d, ND_NOP=%d\n", ND_RETURN, ND_BLOCK, ND_CASE, ND_NOP);
    fclose(f);
    return 0;
}
