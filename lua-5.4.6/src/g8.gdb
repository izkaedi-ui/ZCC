set pagination off
set confirm off
b luaL_getsubtable
b lua_createtable
b luaL_requiref
r < /dev/null
c
c
quit
