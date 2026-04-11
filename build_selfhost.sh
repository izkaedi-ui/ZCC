#!/bin/sh
# Rebuild zcc and self-host.
# IMPORTANT: Never run zcc2 with -o zcc2.s (that overwrites good asm with corrupt output).
set -e
echo "=== Stage 0: concat parts ==="
cat part1.c part2.c part3.c ir.h ir_emit_dispatch.h ir_bridge.h part4.c part5.c ir.c > zcc.c

echo "=== Stage 1: GCC builds zcc, zcc compiles zcc.c -> zcc2.s, link -> zcc2 ==="
gcc -O0 -w -o zcc zcc.c
# Remove old zcc2.s/zcc2 so we never accidentally have corrupt asm from a previous zcc2 run
rm -f zcc2.s zcc2
./zcc zcc.c -o zcc2.s
gcc -O0 -w -o zcc2 zcc2.s

echo "=== Stage 2: zcc2 compiles zcc.c -> zcc3.s (NOT zcc2.s!), link -> zcc3 ==="
./zcc2 zcc.c -o zcc3.s
gcc -O0 -w -o zcc3 zcc3.s

echo "=== Compare stage1 vs stage2 assembly ==="
md5sum zcc2.s zcc3.s
if cmp -s zcc2.s zcc3.s; then
  echo "OK: zcc2.s and zcc3.s match — self-host verified."
else
  echo "MISMATCH: stage2 output differs (or zcc3.s empty if zcc2 crashed)."
  exit 1
fi
