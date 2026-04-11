#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== GDB noopt: what does strcmp receive? ==='
cp compiler_passes.c compiler_passes.c.bak_noopt5

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

echo 'int main(){return 0;}' > /tmp/t1.c
gdb -batch \
  -ex 'break strcmp' \
  -ex 'run /tmp/t1.c -o /tmp/t1.s' \
  -ex 'printf "strcmp arg1 (rdi): %p\n", $rdi' \
  -ex 'printf "strcmp arg2 (rsi): %p\n", $rsi' \
  -ex 'x/s $rdi' \
  -ex 'x/s $rsi' \
  -ex 'bt 5' \
  -ex 'info registers rsp rbp' \
  -ex 'continue' \
  -ex 'printf "strcmp2 arg1: %p\n", $rdi' \
  -ex 'printf "strcmp2 arg2: %p\n", $rsi' \
  -ex 'x/s $rdi' \
  -ex 'x/s $rsi' \
  -ex 'bt 3' \
  -ex 'continue' \
  -ex 'continue' \
  -ex 'continue' \
  -ex 'bt 3' \
  ./zcc3_ir_noopt 2>&1 | grep -v 'Thread\|libthread\|warning\|^$' | head -40

cp compiler_passes.c.bak_noopt5 compiler_passes.c
echo 'Restored.'
echo DONE
