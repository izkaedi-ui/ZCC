#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int filepos;
    int size;
    char name[8];
} filelump_t;

int main() {
    FILE *f = fopen("doom1.wad", "rb");
    char id[4];
    int num, ofs;
    fread(id, 1, 4, f);
    fread(&num, 4, 1, f);
    fread(&ofs, 4, 1, f);
    fseek(f, ofs, SEEK_SET);
    for(int i=0; i<num; i++) {
        filelump_t l;
        fread(&l, sizeof(l), 1, f);
        if (i == 1) {
            printf("lump %d: %.8s size=%d\n", i, l.name, l.size);
        }
    }
    return 0;
}