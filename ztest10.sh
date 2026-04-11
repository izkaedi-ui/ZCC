#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== How many PHI instructions does main for.head have? ==='
echo '--- Count PHI instructions per block for main ---'
LINE=$(grep -n 'case OP_PHI:' compiler_passes.c | tail -1 | cut -d: -f1)
echo "OP_PHI handler at line $LINE"
sed -n "${LINE},$((LINE+15))p" compiler_passes.c

echo ''
echo '=== Show ir_asm_emit_phi_edge_copy in full ==='
LINE=$(grep -n 'ir_asm_emit_phi_edge_copy' compiler_passes.c | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+20))p" compiler_passes.c

echo ''
echo '=== Full OP_ALLOCA handler in emitter ==='
sed -n '4815,4830p' compiler_passes.c

echo ''
echo '=== Quick test: disable Mem2Reg and see if it fixes the bug ==='
echo '--- Check if there is a Mem2Reg enable flag ---'
grep -n 'mem2reg\|Mem2Reg\|MEM2REG' compiler_passes.c | head -10

echo ''
echo '=== Count OP_PHI emissions in zcc3_ir.s for main ==='
LINE=$(grep -n '^main:' zcc3_ir.s | head -1 | cut -d: -f1)
END=$((LINE+2000))
sed -n "${LINE},${END}p" zcc3_ir.s | grep -c 'phi\|PHI' 

echo ''  
echo '=== Show the preheader block (block 40) in zcc3_ir.s ==='
grep -n 'Lir_b_1101_40:' zcc3_ir.s
LINE=$(grep -n '\.Lir_b_1101_40:' zcc3_ir.s | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+30))p" zcc3_ir.s

echo DONE
