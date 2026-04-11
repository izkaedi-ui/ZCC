#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== Build noopt with all fixes, GDB the crash ==='
cp compiler_passes.c compiler_passes.c.bak_noopt7
sed -i '/compute_reachability(fn);/a\
    result->n_blocks = fn->n_blocks;\
    for (uint32_t bi = 0; bi < fn->n_blocks && bi < MAX_BLOCKS; bi++) result->order[bi] = bi;\
    return;' compiler_passes.c

make clean 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc zcc.c
./zcc zcc.c -o zcc2.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2 zcc2.s compiler_passes.c compiler_passes_ir.c -lm
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc3_noopt.s 2>/dev/null
gcc -O0 -w -g -o zcc3_noopt zcc3_noopt.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || gcc -O0 -w -g -o zcc3_noopt zcc3_noopt.s

echo 'int main(){return 42;}' > /tmp/t1.c

gdb -batch \
  -ex 'break main' \
  -ex 'run /tmp/t1.c -o /tmp/t1.s' \
  -ex 'printf "HIT main at %p\n", $pc' \
  -ex 'continue' \
  -ex 'printf "CRASH at %p\n", $pc' \
  -ex 'bt 8' \
  -ex 'printf "rdi=%p rsi=%p rsp=%p rbp=%p\n", $rdi, $rsi, $rsp, $rbp' \
  -ex 'x/s $rdi' \
  -ex 'x/s $rsi' \
  -ex 'x/4i $pc-8' \
  ./zcc3_noopt 2>&1 | grep -v '^; \|^$\|nodes=' | head -25

echo ''
echo '=== Also check: does noopt zcc3 have subq extension? ==='
LINE=$(grep -n '^main:' zcc3_noopt.s | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+15))p" zcc3_noopt.s

echo ''
echo '=== Check r10/r11 save/restore around first strcmp ==='
grep -n 'strcmp\|pushq.*r10\|popq.*r10' zcc3_noopt.s | head -20

cp compiler_passes.c.bak_noopt7 compiler_passes.c
echo 'Restored.'
echo DONE
