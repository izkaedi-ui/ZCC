#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== Test: disable optimization passes to confirm ==='
echo '--- Check for pass disable flags ---'
grep -n 'DISABLE\|SKIP.*PASS\|NO_LICM\|NO_DCE\|NO_MEM2REG\|skip_passes\|run_all_passes' compiler_passes.c | head -15

echo ''
echo '=== Show run_all_passes function ==='
LINE=$(grep -n 'void run_all_passes' compiler_passes.c | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+40))p" compiler_passes.c

echo ''
echo '=== Show full ir_asm_emit_phi_edge_copy again for confirmation ==='
LINE=$(grep -n 'ir_asm_emit_phi_edge_copy' compiler_passes.c | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+25))p" compiler_passes.c

echo ''
echo '=== How many OP_PHI instructions exist in the IR for main? ==='
echo '--- Check if Mem2Reg inserted any PHIs ---'
grep -c 'OP_PHI' /tmp/ir_debug.txt

echo ''
echo '=== Show all OP_BR emissions from for.head that call phi_edge_copy ==='
echo '--- OP_BR handler ---'
grep -n 'case OP_BR:' compiler_passes.c | tail -3
sed -n '4717,4728p' compiler_passes.c

echo DONE
