#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== TARGETED DIAGNOSTIC ==='
echo '--- 1. Does movl still appear in loads/stores? ---'
echo '    movl in OP_LOAD/STORE pattern (deref through pointer):'
grep -c 'movl (%rax)' zcc3_ir.s
grep -c 'movl %ecx, (%rax)' zcc3_ir.s
echo '    movq equivalents:'
grep -c 'movq (%rax), %rax' zcc3_ir.s
grep -c 'movq %rcx, (%rax)' zcc3_ir.s

echo ''
echo '--- 2. Show main for-loop + strcmp area (opt build) ---'
LINE=$(grep -n '^main:' zcc3_ir.s | head -1 | cut -d: -f1)
grep -n 'strcmp' zcc3_ir.s | head -5
SLINE=$(grep -n 'strcmp' zcc3_ir.s | head -1 | cut -d: -f1)
test -n "$SLINE" && sed -n "$((SLINE-25)),$((SLINE+5))p" zcc3_ir.s

echo ''
echo '--- 3. Check for movslq (sign-extension, should be gone) ---'
grep -c 'movslq' zcc3_ir.s
echo '    First 5 movslq occurrences:'
grep -n 'movslq' zcc3_ir.s | head -5

echo ''
echo '--- 4. Build noopt, verify movl is gone there too ---'
cp compiler_passes.c compiler_passes.c.bak_diag
sed -i '/compute_reachability(fn);/a\
    result->n_blocks = fn->n_blocks;\
    for (uint32_t bi = 0; bi < fn->n_blocks && bi < MAX_BLOCKS; bi++) result->order[bi] = bi;\
    return;' compiler_passes.c
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc_d zcc.c
./zcc_d zcc.c -o zcc2_d.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2_d zcc2_d.s compiler_passes.c compiler_passes_ir.c -lm
ZCC_IR_BACKEND=1 ./zcc2_d zcc_pp.c -o noopt_diag.s 2>/dev/null
echo '    NOOPT movl derefs:'
grep -c 'movl (%rax)' noopt_diag.s
grep -c 'movl %ecx, (%rax)' noopt_diag.s
echo '    NOOPT movq derefs:'
grep -c 'movq (%rax), %rax' noopt_diag.s  
grep -c 'movq %rcx, (%rax)' noopt_diag.s
echo '    NOOPT movslq:'
grep -c 'movslq' noopt_diag.s

echo ''
echo '--- 5. Show noopt main strcmp area ---'
SLINE=$(grep -n 'strcmp' noopt_diag.s | head -1 | cut -d: -f1)
test -n "$SLINE" && sed -n "$((SLINE-30)),$((SLINE+5))p" noopt_diag.s

cp compiler_passes.c.bak_diag compiler_passes.c
echo 'Restored.'
echo DONE
