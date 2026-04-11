#!/bin/sh
# Full bootstrap and self-host verify. Run from repo root.
#
# The trick: zcc.c contains things ZCC can't parse (#include, _Static_assert,
# <stdint.h>). That caused parse failure before any assembly — it looked like
# a "stage 2 crash" but the fix is not inside ZCC: build a ZCC-parseable
# zcc_pp.c once in this script, then every ZCC invocation uses zcc_pp.c.
# ZCC never sees zcc.c directly.
#
# Builds zcc from part1..part5 + compiler_passes.c. zcc_pp.c = zcc.c with
# _Static_assert stripped and #include "zcc_ast_bridge.h" replaced by
# zcc_ast_bridge_zcc.h (typedef long int64_t; etc., no <stdint.h>).
# On GCC 14 / Kali: install gcc-12 and run CC=gcc-12 ./run_selfhost.sh
# -B forces this compiler's crt (e.g. gcc-12's Scrt1.o), avoiding GCC 14 ld .sframe error.
# (Older ld does not support -Wl,--discard-sframe; we do not pass it.)
# IR bridge (DCE/LICM in compiler_passes) is off by default — zcc2 + run_all_passes segfaults.
# Optional: ZCC_IR_BRIDGE=1 ./zcc …  (gcc-built zcc only)
set -e
if [ -n "$CC" ] && ! command -v "$CC" >/dev/null 2>&1; then
  echo "Warning: $CC not found, using gcc"
  CC=gcc
fi
CC="${CC:-gcc}"
B_FLAG=""
_scrt=$($CC -print-file-name=Scrt1.o 2>/dev/null) && [ -n "$_scrt" ] && B_FLAG="-B$(dirname "$_scrt")"
LDF="${LDFLAGS_SFRAME:-}"

echo "=== concat parts -> zcc.c ==="
cat part1.c part2.c part3.c ir.h ir_emit_dispatch.h ir_bridge.h part4.c part5.c ir.c ir_to_x86.c > zcc.c

echo "=== $CC -o zcc zcc.c compiler_passes.c compiler_passes_ir.c -lm ==="
$CC -O0 -w -fno-asynchronous-unwind-tables $B_FLAG $LDF -o zcc zcc.c compiler_passes.c compiler_passes_ir.c -lm

echo "=== build zcc_pp.c (ZCC-parseable: strip _Static_assert, inline bridge header, drop system includes) ==="
sed '/^_Static_assert/d' zcc.c > zcc_pp.c
sed -e '/#include "zcc_ast_bridge.h"/r zcc_ast_bridge_zcc.h' -e '/#include "zcc_ast_bridge.h"/d' -e '/#include "zcc_ir_bridge.h"/r zcc_ir_bridge_zcc.h' -e '/#include "zcc_ir_bridge.h"/d' zcc_pp.c > zcc_pp.c.tmp && mv zcc_pp.c.tmp zcc_pp.c
# Drop #include <...> so ZCC never looks for system headers; init_compiler() already provides FILE, fprintf, malloc, etc.
sed '/^#include[[:space:]]*<[^>]*>/d' zcc_pp.c > zcc_pp.c.tmp && mv zcc_pp.c.tmp zcc_pp.c

echo "=== ZCC_IR_BRIDGE=1 ZCC_IR_BACKEND=1 ./zcc zcc_pp.c -o zcc2_bin ==="
ZCC_IR_BRIDGE=1 ZCC_IR_BACKEND=1 ./zcc zcc_pp.c -o zcc2_bin || true

echo "=== $CC -o zcc2 zcc2_bin.s compiler_passes.c compiler_passes_ir.c -lm ==="
$CC -O0 -w -fno-asynchronous-unwind-tables $B_FLAG $LDF -o zcc2 zcc2_bin.s compiler_passes.c compiler_passes_ir.c -lm

echo "=== ZCC_IR_BRIDGE=1 ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc3_bin ==="
ZCC_IR_BRIDGE=1 ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc3_bin || true

if [ -s zcc3_bin.s ]; then
  echo "=== cmp zcc2_bin.s zcc3_bin.s ==="
  cmp zcc2_bin.s zcc3_bin.s
  if cmp -s zcc2_bin.s zcc3_bin.s; then
    echo "SELF-HOST OK"
    exit 0
  fi
  echo "MISMATCH (first 80 diff lines):"
  diff zcc2_bin.s zcc3_bin.s | head -80
  exit 1
else
  echo "FAIL: zcc3.s is empty (zcc2 segfaulted or produced no output)"
  echo ""
  echo "--- GDB backtrace and rdi at crash ---"
  gdb -batch -nx -ex "run" -ex "bt" -ex "p/x \$rdi" -ex "quit" --args ./zcc2 zcc_pp.c -o zcc3.s 2>&1 || true
  echo ""
  echo "--- First 25 lines of next_token in zcc2.s (expect NULL check then je) ---"
  awk '/^next_token:/{p=1} p{print; if(++n>=25) exit}' zcc2.s
  echo ""
  echo "Tip: Full rebuild: rm -f zcc2.s zcc2 zcc3.s zcc_pp.c; ./run_selfhost.sh. If rdi=0 at crash, a caller passed bad cc (e.g. -40) so cc->tk_text was 0. See docs/SELFHOST_RECORD.md."
  exit 1
fi
