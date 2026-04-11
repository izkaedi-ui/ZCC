#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== Build noopt one more time ==='
cp compiler_passes.c compiler_passes.c.bak_final
sed -i '/compute_reachability(fn);/a\
    result->n_blocks = fn->n_blocks;\
    for (uint32_t bi = 0; bi < fn->n_blocks && bi < MAX_BLOCKS; bi++) result->order[bi] = bi;\
    return;' compiler_passes.c
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc_f zcc.c
./zcc_f zcc.c -o zcc2_f.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2_f zcc2_f.s compiler_passes.c compiler_passes_ir.c -lm
ZCC_IR_BACKEND=1 ./zcc2_f zcc_pp.c -o noopt_f.s 2>/dev/null

echo ''
echo '=== 1. Where are the 2 remaining movslq? ==='
grep -n 'movslq' noopt_f.s

echo ''
echo '=== 2. Find main function in noopt ==='
grep -n '^main:' noopt_f.s

echo ''
echo '=== 3. Find strcmp calls in main ==='
MLINE=$(grep -n '^main:' noopt_f.s | head -1 | cut -d: -f1)
MEND=$((MLINE + 4000))
sed -n "${MLINE},${MEND}p" noopt_f.s | grep -n 'strcmp'

echo ''
echo '=== 4. Show 40 lines before first strcmp in main ==='
FIRST_STRCMP=$(sed -n "${MLINE},${MEND}p" noopt_f.s | grep -n 'strcmp' | head -1 | cut -d: -f1)
ABS=$((MLINE + FIRST_STRCMP - 1))
sed -n "$((ABS-40)),${ABS}p" noopt_f.s

echo ''
echo '=== 5. Show the for-loop init and head in main ==='
echo '--- Look for the i=1 constant and loop comparison ---'
sed -n "${MLINE},$((MLINE+100))p" noopt_f.s | grep -n 'movq \$1\|cmpq\|setl\|for\|jmp\|subq'

cp compiler_passes.c.bak_final compiler_passes.c
echo 'Restored.'
echo DONE
