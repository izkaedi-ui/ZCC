#include <stdio.h>
#include "zcc.c"

int zzz() {
    printf("TK_SEMI: %d, TK_LBRACE: %d, TK_RBRACE: %d, TK_LBRACKET: %d, TK_RBRACKET: %d, TK_COMMA: %d\n", TK_SEMI, TK_LBRACE, TK_RBRACE, TK_LBRACKET, TK_RBRACKET, TK_COMMA);
    return 0;
}
int main() { return zzz(); }
