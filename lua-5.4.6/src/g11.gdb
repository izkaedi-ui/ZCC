set pagination off
set confirm off
b luaL_getsubtable
b lua_createtable
b lua_pushvalue
b lua_setfield
b luaL_requiref
r < /dev/null
# inside luaL_getsubtable
c
# inside lua_createtable
set $L = $rdi
printf "\nlua_createtable: L->top.p = 0x%llx\n", *(long long*)($L + 16)
c
# inside lua_pushvalue
set $L = $rdi
printf "\nlua_pushvalue: L->top.p = 0x%llx\n", *(long long*)($L + 16)
c
# inside lua_setfield
set $L = $rdi
printf "\nlua_setfield: L->top.p = 0x%llx\n", *(long long*)($L + 16)
finish
set $L = $rdi
printf "\nAfter lua_setfield: L->top.p = 0x%llx\n", *(long long*)($L + 16)
set $top_tval = *(long long*)($L + 16) - 16
printf "Top stack TValue (L->top-1) type tag: %d\n", *(char*)($top_tval + 8)
c
quit
