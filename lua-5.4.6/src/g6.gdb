set pagination off
set confirm off
b luaL_getsubtable
r < /dev/null
# Finish lua_getfield inside luaL_getsubtable
n
# Now we are AT the condition checking if the return is LUA_TTABLE.
printf "lua_getfield returned: %d\n", $rax
p "Top of stack type should now be clear."
set $L = $rdi
set $top_ptr = *(long long*)($L + 16)
set $tval_ptr = $top_ptr - 16
printf "Top stack TValue at 0x%llx\n", $tval_ptr
x/2gx $tval_ptr
printf "Type tag (offset 8): %d\n", *(int*)($tval_ptr + 8)
c
quit
