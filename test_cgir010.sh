#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '========================================='
echo '  CG-IR-010: 8-BYTE LOAD/STORE FIX'
echo '========================================='
python3 fix_cgir010.py

echo ''
echo '=== Build pipeline ==='
make clean 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc zcc.c
./zcc zcc.c -o zcc2.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2 zcc2.s compiler_passes.c compiler_passes_ir.c -lm
echo 'zcc2 OK'

echo ''
echo '=== NOOPT BUILD (the hard test) ==='
cp compiler_passes.c compiler_passes.c.bak_test010
sed -i '/compute_reachability(fn);/a\
    result->n_blocks = fn->n_blocks;\
    for (uint32_t bi = 0; bi < fn->n_blocks && bi < MAX_BLOCKS; bi++) result->order[bi] = bi;\
    return;' compiler_passes.c
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc_n zcc.c
./zcc_n zcc.c -o zcc2_n.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2_n zcc2_n.s compiler_passes.c compiler_passes_ir.c -lm
ZCC_IR_BACKEND=1 ./zcc2_n zcc_pp.c -o noopt.s 2>/dev/null
gcc -O0 -w -o zcc3_noopt noopt.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || gcc -O0 -w -o zcc3_noopt noopt.s
echo 'int main(){return 42;}' > /tmp/t1.c
./zcc3_noopt /tmp/t1.c -o /tmp/t1.s 2>/tmp/t1.err
RC=$?; L=0; test -f /tmp/t1.s && L=$(wc -l < /tmp/t1.s)
echo "NOOPT trivial: rc=$RC lines=$L"
head -3 /tmp/t1.err
cp compiler_passes.c.bak_test010 compiler_passes.c

echo ''
echo '=== OPT BUILD ==='
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o opt.s 2>/dev/null
gcc -O0 -w -o zcc3_opt opt.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || gcc -O0 -w -o zcc3_opt opt.s
./zcc3_opt /tmp/t1.c -o /tmp/t1_opt.s 2>/tmp/t1_opt.err
RC=$?; L=0; test -f /tmp/t1_opt.s && L=$(wc -l < /tmp/t1_opt.s)
echo "OPT trivial: rc=$RC lines=$L"

echo ''
echo '=== SELF-COMPILE (noopt) ==='
cp compiler_passes.c compiler_passes.c.bak_test010b
sed -i '/compute_reachability(fn);/a\
    result->n_blocks = fn->n_blocks;\
    for (uint32_t bi = 0; bi < fn->n_blocks && bi < MAX_BLOCKS; bi++) result->order[bi] = bi;\
    return;' compiler_passes.c
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc_n2 zcc.c
./zcc_n2 zcc.c -o zcc2_n2.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2_n2 zcc2_n2.s compiler_passes.c compiler_passes_ir.c -lm
ZCC_IR_BACKEND=1 ./zcc2_n2 zcc_pp.c -o noopt2.s 2>/dev/null
gcc -O0 -w -o zcc3_noopt2 noopt2.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || gcc -O0 -w -o zcc3_noopt2 noopt2.s
./zcc3_noopt2 zcc_pp.c -o /tmp/selfcheck.s 2>/tmp/selfcheck.err
RC=$?; L=0; test -f /tmp/selfcheck.s && L=$(wc -l < /tmp/selfcheck.s)
echo "NOOPT self-compile: rc=$RC lines=$L"
head -5 /tmp/selfcheck.err
cp compiler_passes.c.bak_test010b compiler_passes.c

echo ''
echo '========================================='
echo DONE
