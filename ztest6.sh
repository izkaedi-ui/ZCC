#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== ND_FOR handler in compiler_passes.c (around line 3546) ==='
sed -n '3540,3610p' compiler_passes.c

echo ''
echo '=== ND_WHILE handler for comparison ==='
sed -n '2570,2610p' compiler_passes.c

echo ''
echo '=== ZND_FOR in emit_stmt (around line 2580) ==='
sed -n '2575,2640p' compiler_passes.c

echo ''
echo '=== main for-loop assembly in zcc3_ir.s ==='
grep -n 'for\.\|\.Lir_b_1101' zcc3_ir.s | head -30

echo ''
echo '=== Show the comparison block in main ==='
sed -n '34580,34650p' zcc3_ir.s

echo DONE
