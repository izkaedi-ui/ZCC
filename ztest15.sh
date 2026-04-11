#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== GDB the no-opt build ==='
cp compiler_passes.c compiler_passes.c.bak_noopt3

sed -i '/compute_reachability(fn);/a\
    result->n_blocks = fn->n_blocks;\
    for (uint32_t bi = 0; bi < fn->n_blocks && bi < MAX_BLOCKS; bi++) result->order[bi] = bi;\
    return;' compiler_passes.c

make clean 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc zcc.c
./zcc zcc.c -o zcc2.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2 zcc2.s compiler_passes.c compiler_passes_ir.c -lm
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc3_ir_noopt.s 2>/dev/null
gcc -O0 -w -g -o zcc3_ir_noopt zcc3_ir_noopt.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || gcc -O0 -w -g -o zcc3_ir_noopt zcc3_ir_noopt.s
echo 'Built noopt'

echo 'int main(){return 0;}' > /tmp/t1.c
gdb -batch -ex 'run /tmp/t1.c -o /tmp/t1.s' -ex 'bt' -ex 'info registers rsp rbp' -ex 'x/4gx $rsp' ./zcc3_ir_noopt 2>&1 | tail -20

echo ''
echo '=== Show full OP_CALL handler cleanup (after callq) ==='
cp compiler_passes.c.bak_noopt3 compiler_passes.c
sed -n '4825,4870p' compiler_passes.c

echo DONE
