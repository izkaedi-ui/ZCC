#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== Full IR dump for main (before and after passes) ==='
echo 'int main(){return 0;}' > /tmp/t1.c

# Enable full debug for main
ZCC_IR_BACKEND=1 ZCC_PGO_DEBUG_MAIN=1 ./zcc2_ast zcc_pp.c -o /tmp/dbg.s 2>/tmp/ir_debug.txt

echo '=== RAW IR blocks for main ==='
grep -A 200 'RAW IR FOR MAIN' /tmp/ir_debug.txt | head -220

echo ''
echo '=== POST-PASS IR for main ==='  
grep -A 300 'PGO-DEBUG-IR.*=== main' /tmp/ir_debug.txt | head -320

echo ''
echo '=== Alloca offsets for main ==='
grep -i 'alloca\|main.*block\|main.*emit' /tmp/ir_debug.txt | head -30

echo DONE
