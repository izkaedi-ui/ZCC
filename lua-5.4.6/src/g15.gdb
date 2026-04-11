set pagination off
set confirm off
b luaL_requiref
r < /dev/null
b lua_getfield
commands
  printf "[lua_getfield] idx=%d string=%s\n", $rsi, $rdx
  printf "[lua_getfield] L->top.p=0x%llx\n", *(long long*)($rdi+16)
  c
end
b lua_createtable
commands
  printf "[lua_createtable] L->top.p=0x%llx\n", *(long long*)($rdi+16)
  c
end
b lua_setfield
commands
  printf "[lua_setfield] idx=%d string=%s\n", $rsi, $rdx
  printf "[lua_setfield] L->top.p=0x%llx\n", *(long long*)($rdi+16)
  c
end
c
c
quit
