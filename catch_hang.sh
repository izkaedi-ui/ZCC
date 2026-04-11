#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux
make clean
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc zcc.c compiler_passes.c compiler_passes_ir.c -lm
./zcc zcc.c -o zcc2.s
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2 zcc2.s compiler_passes.c compiler_passes_ir.c -lm
./zcc2 zcc.c -o zcc3.s 2> test_debug.log &
PID=$!
echo "Running hanging process with PID $PID"
sleep 5
gdb -batch -p $PID \
    -ex 'bt' \
    -ex 'info locals' \
    -ex 'info registers rip rsp rbp' \
    -ex 'x/10i $rip' \
    -ex 'quit' > hang_bt.txt 2>&1
kill -9 $PID 2>/dev/null
echo "Done"
