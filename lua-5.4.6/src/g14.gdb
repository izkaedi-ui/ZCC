set pagination off
set confirm off
b luaL_requiref
r < /dev/null
b lua_getfield
commands
  printf "\n=== lua_getfield ===\n"
  printf "idx: %d\n", $rsi
  printf "key: %s\n", $rdx
  set $L = (lua_State*)$rdi
  printf "L->top.p = 0x%llx\n", $L->top.p
  printf "TValue at L->top.p-1 type: %d\n", ($L->top.p - 1)->tt_
  c
end
c
