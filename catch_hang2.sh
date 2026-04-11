#!/bin/bash
./zcc2 t.c -o t.s > /dev/null 2>&1 &
PID=$!
sleep 2
echo "Attaching to $PID"
gdb -batch -p $PID \
    -ex 'bt' \
    -ex 'info locals' \
    -ex 'info registers rip rsp rbp' \
    -ex 'x/10i $rip' \
    -ex 'quit' > hang_bt.txt 2>&1
kill -9 $PID 2>/dev/null
echo "Got backtrace for $PID"
