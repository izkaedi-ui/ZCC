#!/usr/bin/env bash
# Build Lua amalgam → ZCC compile → runtime barrier-escape test

set -euo pipefail

LUA_SRC="/mnt/h/__DOWNLOADS/selforglinux/lua-5.4.6/src"
ZCC2="/mnt/h/__DOWNLOADS/selforglinux/zcc2"
PREP="/mnt/h/__DOWNLOADS/selforglinux/prep_lua_for_zcc.py"
OUT="/mnt/h/__DOWNLOADS/selforglinux"

LUA_SRCS="lapi.c lauxlib.c lbaselib.c lcode.c lcorolib.c lctype.c \
          ldblib.c ldebug.c ldo.c ldump.c lfunc.c lgc.c linit.c liolib.c \
          llex.c lmathlib.c lmem.c loadlib.c lobject.c lopcodes.c loslib.c \
          lparser.c lstate.c lstring.c lstrlib.c ltable.c ltablib.c \
          ltm.c lundump.c lutf8lib.c lvm.c lzio.c"

echo "=== STEP 1: GCC preprocess Lua → lua_amalg_raw.i ==="
cd "$LUA_SRC"
gcc -E -DLUA_USE_LINUX -DLUA_COMPAT_5_3 -I. $LUA_SRCS > "$OUT/lua_amalg_raw.i" 2>&1
echo "Lines: $(wc -l < $OUT/lua_amalg_raw.i)"

echo
echo "=== STEP 2: prep_lua_for_zcc.py → lua_zcc.c ==="
python3 "$PREP" "$OUT/lua_amalg_raw.i" "$OUT/lua_zcc.c"

echo
echo "=== STEP 3: ZCC compile lua_zcc.c → lua_zcc.s ==="
cd "$OUT"
"$ZCC2" lua_zcc.c -o lua_zcc.s 2>&1
echo "ASM lines: $(wc -l < lua_zcc.s)"

echo
echo "=== STEP 4: GCC link → lua_boot ==="
gcc -O0 -w -fno-asynchronous-unwind-tables -no-pie -o lua_boot \
    lua_zcc.s lua.c -lm -ldl 2>&1

echo
echo "=== STEP 5: Runtime barrier-escape test ==="
echo 'print("barrier_escape: OK")' | timeout 5 ./lua_boot 2>&1 && echo "PASS" || echo "FAIL"

echo
echo "=== STEP 6: luaL_checkversion_ test ==="
echo 'local t = {} for i=1,10 do t[i] = i end print(t[5])' | timeout 5 ./lua_boot 2>&1 && echo "PASS" || echo "FAIL"
