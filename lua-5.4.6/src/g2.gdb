set pagination off
set confirm off
b luaL_requiref
r < /dev/null
bt 6
# rdi = lua_State*, rsi = modname string, rdx = open fn ptr, ecx = glb int
# Inspect the TValue at L->l_gt (the global table) — it's at fixed offset in global_State
# L points to lua_State. global_State is accessible via L->l_G
# In Lua 5.4, G(L) = cast(global_State*, (L->l_G))
# l_registry is a TValue at offset 0 in global_State
# Let's dump 32 bytes from the lua_State and find l_G
x/20gx $rdi
quit
