#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== Test: skip all optimization passes ==='
echo '--- Temporarily patch run_all_passes to return early ---'
cp compiler_passes.c compiler_passes.c.bak_passtest

# Find run_all_passes and add early return after reachability
sed -i '/compute_reachability(fn);/a\
    /* TEMP: skip all passes for diagnosis */\
    result->n_blocks = fn->n_blocks;\
    for (uint32_t bi = 0; bi < fn->n_blocks && bi < MAX_BLOCKS; bi++) result->order[bi] = bi;\
    return;' compiler_passes.c

echo 'Patched. Rebuilding...'
make clean 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc zcc.c
./zcc zcc.c -o zcc2.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2 zcc2.s compiler_passes.c compiler_passes_ir.c -lm
echo 'Built zcc2 with no-opt passes'

echo ''
echo '=== Stage 4: Build zcc3_ir with NO optimization passes ==='
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc3_ir_noopt.s 2>/tmp/noopt_err.txt
RC=$?
L=0
test -f zcc3_ir_noopt.s && L=$(wc -l < zcc3_ir_noopt.s)
echo "IR build: rc=$RC lines=$L"

echo ''
echo '=== Link and test ==='
gcc -O0 -w -o zcc3_ir_noopt zcc3_ir_noopt.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || gcc -O0 -w -o zcc3_ir_noopt zcc3_ir_noopt.s
echo 'int main(){return 0;}' > /tmp/t1.c
./zcc3_ir_noopt /tmp/t1.c -o /tmp/t1_noopt.s 2>/tmp/t1_noopt_err.txt
RC=$?
L=0
test -f /tmp/t1_noopt.s && L=$(wc -l < /tmp/t1_noopt.s)
echo "trivial test: rc=$RC lines=$L"

echo ''
echo '=== Full self-compile test ==='
./zcc3_ir_noopt zcc_pp.c -o /tmp/check_noopt.s 2>/tmp/check_noopt_err.txt
RC=$?
L=0
test -f /tmp/check_noopt.s && L=$(wc -l < /tmp/check_noopt.s)
echo "self-compile: rc=$RC lines=$L"
head -5 /tmp/check_noopt_err.txt

echo ''
echo '=== Restore original ==='
cp compiler_passes.c.bak_passtest compiler_passes.c
echo 'Restored.'

echo DONE
