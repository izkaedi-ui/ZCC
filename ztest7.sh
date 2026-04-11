#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== ir_asm_load_to_rax function ==='
grep -n 'ir_asm_load_to_rax' compiler_passes.c | head -5
LINE=$(grep -n 'static void ir_asm_load_to_rax' compiler_passes.c | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+30))p" compiler_passes.c

echo ''
echo '=== ir_asm_store_rax_to function ==='
LINE=$(grep -n 'static void ir_asm_store_rax_to' compiler_passes.c | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+30))p" compiler_passes.c

echo ''
echo '=== OP_LOAD handler ==='
grep -n 'case OP_LOAD' compiler_passes.c | head -5
LINE=$(grep -n 'case OP_LOAD:' compiler_passes.c | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+25))p" compiler_passes.c

echo ''
echo '=== OP_STORE handler ==='
LINE=$(grep -n 'case OP_STORE:' compiler_passes.c | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+25))p" compiler_passes.c

echo ''
echo '=== OP_ALLOCA handler ==='
LINE=$(grep -n 'case OP_ALLOCA:' compiler_passes.c | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+20))p" compiler_passes.c

echo ''
echo '=== OP_LT handler (the comparison) ==='
LINE=$(grep -n 'case OP_LT:' compiler_passes.c | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+15))p" compiler_passes.c

echo ''
echo '=== ir_asm_emit_src_operand ==='
LINE=$(grep -n 'ir_asm_emit_src_operand' compiler_passes.c | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+20))p" compiler_passes.c

echo DONE
