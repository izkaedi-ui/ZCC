#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux
make clean 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc zcc.c
./zcc zcc.c -o zcc2.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2 zcc2.s compiler_passes.c compiler_passes_ir.c -lm
echo 'Built zcc2'
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc3_ir.s 2>/tmp/s4.txt
tail -5 /tmp/s4.txt
gcc -O0 -w -o zcc3_ir zcc3_ir.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || gcc -O0 -w -o zcc3_ir zcc3_ir.s
echo 'Built zcc3_ir'
echo 'int main(){return 0;}' > /tmp/t1.c
./zcc3_ir /tmp/t1.c -o /tmp/t1.s 2>/tmp/t1.err
RC=$?
L=0
test -f /tmp/t1.s && L=$(wc -l < /tmp/t1.s)
echo "trivial: rc=$RC lines=$L"
./zcc3_ir zcc_pp.c -o /tmp/check.s 2>/tmp/check.err
RC=$?
L=0
test -f /tmp/check.s && L=$(wc -l < /tmp/check.s)
echo "self-compile: rc=$RC lines=$L"
head -5 /tmp/check.err
echo DONE
