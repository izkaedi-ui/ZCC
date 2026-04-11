set pagination off
set confirm off
b luaL_getsubtable
b lua_absindex
r < /dev/null
c
c
printf "\n=== In lua_absindex ===\n"
printf "idx arg: %d\n", $rsi
finish
printf "lua_absindex returned: %d\n", $rax
quit
