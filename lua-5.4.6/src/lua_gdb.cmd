set pagination off
set -std-value-printing off
b luaL_requiref
b lua_getfield
b luaG_typeerror
r <<< 'print(1+1)'
