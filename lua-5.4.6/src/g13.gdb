set pagination off
set confirm off
b luaL_requiref
r < /dev/null
b luaL_getsubtable
c
# finish luaL_getsubtable
finish
printf "\n=== Returned to luaL_requiref ===\n"
set $L = $rdi
set $top_ptr = *(long long*)($L + 16)
printf "L->top.p = 0x%llx\n", $top_ptr
printf "TValue at L->top.p-1 (offset -16): type=%d\n", *(char*)($top_ptr - 16 + 8)

b lua_getfield
c
printf "\n=== Inside lua_getfield (first time after getsubtable) ===\n"
set $L = $rdi
set $top_ptr = *(long long*)($L + 16)
printf "idx: %d\n", $rsi
printf "string key: %s\n", $rdx
printf "L->top.p = 0x%llx\n", $top_ptr
printf "TValue at L->top.p-1 (offset -16): type=%d\n", *(char*)($top_ptr - 16 + 8)
quit
