#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== STRACE: what syscalls does zcc3_ir make? ==='
echo 'int main(){return 0;}' > /tmp/t1.c
strace -f -e trace=open,openat,read,write,exit_group ./zcc3_ir /tmp/t1.c -o /tmp/t1.s 2>/tmp/strace_out.txt
echo "exit=$?"
echo '=== Last 30 lines of strace ==='
tail -30 /tmp/strace_out.txt
echo ''
echo '=== Did it open the input file? ==='
grep 't1.c' /tmp/strace_out.txt
echo ''
echo '=== Did it open the output file? ==='
grep 't1.s' /tmp/strace_out.txt
echo ''
echo '=== How did it exit? ==='
grep 'exit_group' /tmp/strace_out.txt
echo ''
echo '=== Compare: what does reference zcc do? ==='
strace -f -e trace=open,openat,read,write,exit_group ./zcc /tmp/t1.c -o /tmp/t1_ref.s 2>/tmp/strace_ref.txt
echo "ref_exit=$?"
echo '=== Ref: last 10 strace lines ==='
tail -10 /tmp/strace_ref.txt

echo DONE
