#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== OP_ALLOCA handler in the emitter ==='
grep -n 'case OP_ALLOCA' compiler_passes.c | head -5
sed -n '4660,4690p' compiler_passes.c

echo ''
echo '=== ir_asm_slot and vreg_location ==='
grep -n 'ir_asm_slot\|alloca_off' compiler_passes.c | head -15

echo ''
echo '=== Full ir_asm_load_to_rax - does it check alloca_off? ==='
sed -n '4545,4560p' compiler_passes.c

echo ''
echo '=== Reference main: for-loop region (AST output) ==='
echo '--- Lines around the for (i=1; i<argc; i++) ---'
grep -n 'movl.*1.*348\|cmp.*354\|jl \|movq.*rdi\|strcmp' reference.s | head -20

echo ''
echo '=== Reference main: show the actual comparison ==='
sed -n '47130,47180p' reference.s

echo ''
echo '=== IR main: the block that jumps to .Lir_b_1101_2 (for.head) ==='
grep -n 'Lir_b_1101_2:' zcc3_ir.s
LINE=$(grep -n '\.Lir_b_1101_2:' zcc3_ir.s | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+40))p" zcc3_ir.s

echo ''
echo '=== How many .Lir_b_1101_ blocks exist? ==='
grep -c '\.Lir_b_1101_' zcc3_ir.s

echo DONE
