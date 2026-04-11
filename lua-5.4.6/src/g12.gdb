set pagination off
set confirm off
b lua_createtable
commands
  printf "\n[lua_createtable entry] L->top.p = 0x%llx\n", *(long long*)($rdi + 16)
  c
end

b lua_pushvalue
commands
  printf "\n[lua_pushvalue entry] L->top.p = 0x%llx\n", *(long long*)($rdi + 16)
  c
end

b lua_setfield
commands
  printf "\n[lua_setfield entry] L->top.p = 0x%llx\n", *(long long*)($rdi + 16)
  
  # since lua_setfield crashes, we shouldn't continue; let's inspect the stack!
  set $L = $rdi
  set $top_ptr = *(long long*)($L + 16)
  printf "TValue at L->top.p-1 (offset -16): type=%d\n", *(char*)($top_ptr - 16 + 8)
  printf "TValue at L->top.p-2 (offset -32): type=%d\n", *(char*)($top_ptr - 32 + 8)
  printf "TValue at L->top.p-3 (offset -48): type=%d\n", *(char*)($top_ptr - 48 + 8)
  # step into lua_setfield to see what it does
  s
  s
  s
  s
  s
  printf "\n[inside lua_setfield] L->top.p = 0x%llx\n", *(long long*)($L + 16)
  quit
end

b luaG_typeerror
commands
  printf "\n[CRASH] luaG_typeerror!\n"
  quit
end

r < /dev/null
quit
