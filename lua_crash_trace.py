#!/usr/bin/env python3
"""
lua_crash_trace.py — automated GDB session to extract crash evidence from
the ZCC-compiled lua_boot binary.

Goal: identify which table is nil in luaL_requiref -> lua_getfield path.
"""
import subprocess
import sys

GDB_SCRIPT = r"""
set pagination off
set confirm off
set backtrace limit 20

# Break at key functions
b luaL_requiref
b luaG_typeerror

# Run with stdin redirect
run < /dev/null

# On first hit (luaL_requiref) — print args
commands 1
  silent
  printf "\n[GDB] HIT luaL_requiref\n"
  printf "[GDB] L=%p\n", L
  printf "[GDB] modname=%s\n", modname
  printf "[GDB] L->top=%p\n", L->top
  printf "[GDB] L->ci=%p\n", L->ci
  printf "[GDB] G(L)->l_registry type=%d\n", G(L)->l_registry.tt_
  bt 5
  c
end

# On typeerror — this is the crash point
commands 2
  silent
  printf "\n[GDB] HIT luaG_typeerror — CRASH SITE\n"
  printf "[GDB] L=%p\n", L
  bt 15
  frame 0
  info args
  info locals
  quit
end

c
"""

result = subprocess.run(
    ["gdb", "-batch", "-ex", GDB_SCRIPT.strip(), "--args",
     "./lua_boot"],
    capture_output=True, text=True, timeout=30,
    cwd="/mnt/h/__DOWNLOADS/selforglinux/lua-5.4.6/src"
)

print("=== STDOUT ===")
print(result.stdout[-8000:] if len(result.stdout) > 8000 else result.stdout)
print("=== STDERR ===")
print(result.stderr[-3000:] if len(result.stderr) > 3000 else result.stderr)
print("=== RETURN CODE:", result.returncode, "===")
