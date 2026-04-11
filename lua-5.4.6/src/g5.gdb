set pagination off
set confirm off
b luaL_requiref
r < /dev/null
b lua_getfield
commands 2
  printf "\n=== lua_getfield ===\n"
  printf "idx: %d\n", $rsi
  printf "key: %s\n", $rdx
  set $L = $rdi
  set $top_ptr = *(long long*)($L + 16)
  set $tval_ptr = $top_ptr - 16
  printf "Top stack type (offset 8): %d\n", *(int*)($tval_ptr + 8)
  c
end
c
