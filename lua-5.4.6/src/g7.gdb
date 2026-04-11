set pagination off
set confirm off
b luaL_getsubtable
b lua_newtable
b luaL_requiref
r < /dev/null
c
quit
