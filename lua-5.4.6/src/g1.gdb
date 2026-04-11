set pagination off
set confirm off
b luaL_getsubtable
r < /dev/null
bt 5
info args
info locals
p lua_type($rdi, -1)
x/16gx $rdi
quit
