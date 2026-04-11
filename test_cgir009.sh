#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '========================================='
echo '  CG-IR-009 FINAL FIX TEST'  
echo '========================================='
python3 fix_cgir009.py

echo ''
echo '=== Build pipeline ==='
make clean 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc zcc.c
./zcc zcc.c -o zcc2.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2 zcc2.s compiler_passes.c compiler_passes_ir.c -lm
echo 'zcc2 built'

echo ''
echo '=== BUILD OPTIMIZED zcc3_ir ==='
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc3_ir.s 2>/tmp/s4opt.txt
gcc -O0 -w -o zcc3_ir zcc3_ir.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || gcc -O0 -w -o zcc3_ir zcc3_ir.s
echo 'zcc3_ir (opt) built'
echo 'int main(){return 0;}' > /tmp/t1.c
./zcc3_ir /tmp/t1.c -o /tmp/t1_opt.s 2>/tmp/t1_opt.err
RC=$?; L=0; test -f /tmp/t1_opt.s && L=$(wc -l < /tmp/t1_opt.s)
echo "OPT trivial: rc=$RC lines=$L"
./zcc3_ir zcc_pp.c -o /tmp/check_opt.s 2>/tmp/check_opt.err
RC=$?; L=0; test -f /tmp/check_opt.s && L=$(wc -l < /tmp/check_opt.s)
echo "OPT self-compile: rc=$RC lines=$L"

echo ''
echo '=== BUILD NO-OPT zcc3_ir ==='
cp compiler_passes.c compiler_passes.c.bak_noopt_final
sed -i '/compute_reachability(fn);/a\
    result->n_blocks = fn->n_blocks;\
    for (uint32_t bi = 0; bi < fn->n_blocks && bi < MAX_BLOCKS; bi++) result->order[bi] = bi;\
    return;' compiler_passes.c
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc_noopt zcc.c
./zcc_noopt zcc.c -o zcc2_noopt.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2_noopt zcc2_noopt.s compiler_passes.c compiler_passes_ir.c -lm
ZCC_IR_BACKEND=1 ./zcc2_noopt zcc_pp.c -o zcc3_ir_noopt.s 2>/dev/null
gcc -O0 -w -o zcc3_ir_noopt zcc3_ir_noopt.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || gcc -O0 -w -o zcc3_ir_noopt zcc3_ir_noopt.s
echo 'zcc3_ir (noopt) built'
./zcc3_ir_noopt /tmp/t1.c -o /tmp/t1_noopt.s 2>/tmp/t1_noopt.err
RC=$?; L=0; test -f /tmp/t1_noopt.s && L=$(wc -l < /tmp/t1_noopt.s)
echo "NOOPT trivial: rc=$RC lines=$L"
./zcc3_ir_noopt zcc_pp.c -o /tmp/check_noopt.s 2>/tmp/check_noopt.err
RC=$?; L=0; test -f /tmp/check_noopt.s && L=$(wc -l < /tmp/check_noopt.s)
echo "NOOPT self-compile: rc=$RC lines=$L"
head -3 /tmp/check_noopt.err

echo ''
echo '=== Frame extension check ==='
LINE=$(grep -n '^main:' zcc3_ir.s | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+12))p" zcc3_ir.s

cp compiler_passes.c.bak_noopt_final compiler_passes.c
echo 'Restored.'
echo ''
echo '========================================='
echo '  SUMMARY'
echo '========================================='
echo DONE
