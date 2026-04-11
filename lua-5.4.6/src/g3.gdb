set pagination off
set confirm off
b luaL_requiref
r < /dev/null
set $L = $rdi
set $lG = *(long long*)($L + 24)
printf "=== lua_State at 0x%llx ===\n", $L
printf "  l_G ptr (offset 24): 0x%llx\n", $lG
x/2gx ($lG + 64)
printf "  l_registry->tt_ at lG+72: %d\n", *(int*)($lG + 72)
x/1gx ($lG + 264)
printf "=== stack top ===\n"
x/4gx $rdi
quit
