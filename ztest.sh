#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo TEST1_TRIVIAL
echo 'int main(){return 0;}' > /tmp/t1.c
./zcc3_ir /tmp/t1.c -o /tmp/t1.s 2>/tmp/t1.err
RC=$?
L=0
test -f /tmp/t1.s && L=$(wc -l < /tmp/t1.s)
echo "rc=$RC lines=$L"

echo TEST_FULL
./zcc3_ir zcc_pp.c -o /tmp/t3.s 2>/tmp/t3.err
RC=$?
L=0
test -f /tmp/t3.s && L=$(wc -l < /tmp/t3.s)
echo "rc=$RC lines=$L"
head -5 /tmp/t3.err

echo TEST_1000
head -1000 zcc_pp.c > /tmp/t5.c
./zcc3_ir /tmp/t5.c -o /tmp/t5.s 2>/tmp/t5.err
RC=$?
L=0
test -f /tmp/t5.s && L=$(wc -l < /tmp/t5.s)
echo "rc=$RC lines=$L"

echo TEST_3000
head -3000 zcc_pp.c > /tmp/t6.c
./zcc3_ir /tmp/t6.c -o /tmp/t6.s 2>/tmp/t6.err
RC=$?
L=0
test -f /tmp/t6.s && L=$(wc -l < /tmp/t6.s)
echo "rc=$RC lines=$L"

echo TEST_5000
head -5000 zcc_pp.c > /tmp/t7.c
./zcc3_ir /tmp/t7.c -o /tmp/t7.s 2>/tmp/t7.err
RC=$?
L=0
test -f /tmp/t7.s && L=$(wc -l < /tmp/t7.s)
echo "rc=$RC lines=$L"

echo TEST_7338
./zcc3_ir zcc_pp.c -o /tmp/t8.s 2>/tmp/t8.err
RC=$?
L=0
test -f /tmp/t8.s && L=$(wc -l < /tmp/t8.s)
E=$(wc -l < /tmp/t8.err)
echo "rc=$RC lines=$L err_lines=$E"
cat /tmp/t8.err | head -20

echo DONE
