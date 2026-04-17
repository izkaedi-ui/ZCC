#include <stdio.h>

struct Flags {
    unsigned int read  : 1;
    unsigned int write : 1;
    unsigned int exec  : 1;
    unsigned int pad   : 5;
};

int main(void) {
    struct Flags f;
    f.read = 1;
    f.write = 0;
    f.exec = 1;
    
    printf("sizeof(Flags) = %lu\n", (unsigned long)sizeof(struct Flags));
    printf("read=%u write=%u exec=%u\n", f.read, f.write, f.exec);
    
    return 0;
}
