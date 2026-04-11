#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== Check if CG-IR-006 subq appears in the assembly ==='
echo '--- Look at main function start in zcc3_ir.s (with passes) ---'
LINE=$(grep -n '^main:' zcc3_ir.s | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+15))p" zcc3_ir.s

echo ''
echo '=== Count ALL subq in main ==='
END=$((LINE+2000))
sed -n "${LINE},${END}p" zcc3_ir.s | grep 'subq'

echo ''
echo '=== What does fn->n_regs look like for main? ==='
echo '--- Add debug print to see n_regs ---'
grep -n 'ir_need\|n_regs\|frame extension\|CG-IR-006' compiler_passes.c | head -10

echo ''
echo '=== Check alloca_off assignment - what is the deepest offset? ==='
sed -n '4865,4885p' compiler_passes.c

echo ''
echo '=== Show the EXACT code around the frame extension ==='
grep -n 'CG-IR-006\|ir_need\|subq.*extra' compiler_passes.c | head -5
LINE2=$(grep -n 'CG-IR-006' compiler_passes.c | head -1 | cut -d: -f1)
test -n "$LINE2" && sed -n "$((LINE2-2)),$((LINE2+12))p" compiler_passes.c

echo ''
echo '=== Verify (void)stack_size is removed ==='
grep -n 'void.*stack_size' compiler_passes.c | head -5

echo DONE
