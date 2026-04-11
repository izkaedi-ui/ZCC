#include <stdio.h>
typedef unsigned Bool;
struct Test {
    unsigned char a;   // offset 0, size 1
    unsigned char b;   // offset 1
    unsigned char c;   // offset 2
    unsigned char d;   // offset 3
    unsigned char e;   // offset 4
    Bool isEphemeral:1;    // bitfield
    Bool useRandomRowid:1;
    Bool isOrdered:1;
    Bool noReuse:1;
    Bool colCache:1;
    unsigned short seekHit;
};
int main(){
    printf("a=%zu b=%zu c=%zu d=%zu e=%zu\n",
        __builtin_offsetof(struct Test, a),
        __builtin_offsetof(struct Test, b),
        __builtin_offsetof(struct Test, c),
        __builtin_offsetof(struct Test, d),
        __builtin_offsetof(struct Test, e));
    printf("seekHit=%zu  sizeof=%zu\n",
        __builtin_offsetof(struct Test, seekHit),
        sizeof(struct Test));
    return 0;
}
