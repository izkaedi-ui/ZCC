#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== Test no-opt build with all 3 fixes (CG-IR-004/006/007) ==='
cp compiler_passes.c compiler_passes.c.bak_noopt4

sed -i '/compute_reachability(fn);/a\
    result->n_blocks = fn->n_blocks;\
    for (uint32_t bi = 0; bi < fn->n_blocks && bi < MAX_BLOCKS; bi++) result->order[bi] = bi;\
    return;' compiler_passes.c

make clean 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc zcc.c
./zcc zcc.c -o zcc2.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2 zcc2.s compiler_passes.c compiler_passes_ir.c -lm
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc3_ir_noopt.s 2>/dev/null
gcc -O0 -w -o zcc3_ir_noopt zcc3_ir_noopt.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || gcc -O0 -w -o zcc3_ir_noopt zcc3_ir_noopt.s
echo 'Built noopt with all fixes'

echo ''
echo '=== trivial ==='
echo 'int main(){return 0;}' > /tmp/t1.c
./zcc3_ir_noopt /tmp/t1.c -o /tmp/t1.s 2>/tmp/t1.err
RC=$?
L=0
test -f /tmp/t1.s && L=$(wc -l < /tmp/t1.s)
echo "rc=$RC lines=$L"
head -3 /tmp/t1.err

echo ''
echo '=== self-compile ==='
./zcc3_ir_noopt zcc_pp.c -o /tmp/noopt_check.s 2>/tmp/noopt_check.err
RC=$?
L=0
test -f /tmp/noopt_check.s && L=$(wc -l < /tmp/noopt_check.s)
echo "rc=$RC lines=$L"
head -5 /tmp/noopt_check.err

cp compiler_passes.c.bak_noopt4 compiler_passes.c
echo 'Restored.'
echo DONE
