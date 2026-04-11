/* test_lua_boot.c — minimal Lua boot harness for GDB tracing */
#include <stdio.h>
#include <stdlib.h>

/* Lua C API headers — compiled from ZCC output, linked with system gcc */
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int main(void) {
    fprintf(stderr, "[BOOT] Creating Lua state\n");
    lua_State *L = luaL_newstate();
    if (!L) {
        fprintf(stderr, "[FAIL] luaL_newstate returned NULL\n");
        return 1;
    }
    fprintf(stderr, "[BOOT] lua_State created: %p\n", (void*)L);
    fprintf(stderr, "[BOOT] Calling lua_gettop: %d\n", lua_gettop(L));

    /* Test global table is valid before openlibs */
    fprintf(stderr, "[BOOT] Pushing _G check\n");
    lua_getglobal(L, "_G");
    fprintf(stderr, "[BOOT] _G type = %d (5=table, 0=nil)\n", lua_type(L, -1));
    lua_pop(L, 1);

    fprintf(stderr, "[BOOT] Calling luaL_openlibs\n");
    luaL_openlibs(L);
    fprintf(stderr, "[BOOT] luaL_openlibs returned\n");

    lua_close(L);
    fprintf(stderr, "[BOOT] Done\n");
    return 0;
}
