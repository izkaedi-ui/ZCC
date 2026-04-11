#include <stdio.h>

static const char magic[] = "SQLite format 3";

int main() {
    printf("sz=%d\n", (int)sizeof(magic));
    return 0;
}
