#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '========================================='
echo '  CG-IR-008 ROOT CAUSE FIX TEST'
echo '========================================='

echo ''
echo '=== Apply fix ==='
python3 fix_cgir008.py

echo ''
echo '=== Build zcc -> zcc2 ==='
make clean 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc zcc.c
./zcc zcc.c -o zcc2.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2 zcc2.s compiler_passes.c compiler_passes_ir.c -lm
echo 'zcc2 OK'

echo ''
echo '=== Build zcc3_ir (with all passes) ==='
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc3_ir.s 2>/tmp/s4.txt
tail -3 /tmp/s4.txt
gcc -O0 -w -o zcc3_ir zcc3_ir.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || gcc -O0 -w -o zcc3_ir zcc3_ir.s
echo 'zcc3_ir built'

echo ''
echo '=== Test 1: trivial program ==='
echo 'int main(){return 0;}' > /tmp/t1.c
./zcc3_ir /tmp/t1.c -o /tmp/t1.s 2>/tmp/t1.err
RC=$?
L=0
test -f /tmp/t1.s && L=$(wc -l < /tmp/t1.s)
echo "rc=$RC lines=$L"
head -3 /tmp/t1.err

echo ''
echo '=== Test 2: self-compile ==='
./zcc3_ir zcc_pp.c -o /tmp/check.s 2>/tmp/check.err
RC=$?
L=0
test -f /tmp/check.s && L=$(wc -l < /tmp/check.s)
echo "rc=$RC lines=$L"
head -5 /tmp/check.err

echo ''
echo '=== Test 3: GDB crash check ==='
echo 'int main(){return 0;}' > /tmp/t1.c
gdb -batch -ex 'run /tmp/t1.c -o /tmp/t1.s' -ex 'bt 3' ./zcc3_ir 2>&1 | tail -5

echo ''
echo '=== Verify main prologue has frame extension ==='
LINE=$(grep -n '^main:' zcc3_ir.s | head -1 | cut -d: -f1)
sed -n "${LINE},$((LINE+15))p" zcc3_ir.s

echo ''
echo '========================================='
echo '  RESULTS SUMMARY'
echo '========================================='
echo DONE
