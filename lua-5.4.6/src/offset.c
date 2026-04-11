#include <stdio.h>
#include "lstate.h"
int main(void) {
    printf("sizeof lua_State=%zu\n", sizeof(lua_State));
    printf("offset l_G=%zu\n", __builtin_offsetof(lua_State, l_G));
    printf("offset top=%zu\n", __builtin_offsetof(lua_State, top));
    printf("offset ci=%zu\n", __builtin_offsetof(lua_State, ci));
    printf("offset stack=%zu\n", __builtin_offsetof(lua_State, stack));
    printf("sizeof global_State=%zu\n", sizeof(global_State));
    printf("offset l_registry=%zu\n", __builtin_offsetof(global_State, l_registry));
    printf("offset mainthread=%zu\n", __builtin_offsetof(global_State, mainthread));
    return 0;
}
