#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== OP_LOAD/OP_STORE in the EMITTER (ir_asm_emit_one_block) ==='
sed -n '4689,4730p' compiler_passes.c

echo ''
echo '=== OP_LT in the EMITTER ==='
grep -n 'case OP_LT' compiler_passes.c | head -5
sed -n '4736,4760p' compiler_passes.c

echo ''
echo '=== main for.head block in zcc3_ir.s (the comparison block) ==='
echo '--- looking for the block that does the i < argc comparison ---'
sed -n '33850,33920p' zcc3_ir.s

echo ''
echo '=== main init block (where i=1 is set and argv is saved) ==='
sed -n '33700,33760p' zcc3_ir.s

echo ''
echo '=== Full main function assembly (first 200 lines) ==='
grep -n 'main:' zcc3_ir.s
LINE=$(grep -n '^main:' zcc3_ir.s | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+200))p" zcc3_ir.s

echo DONE
