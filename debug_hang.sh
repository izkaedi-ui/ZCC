#!/bin/bash
pid=$(pgrep -f '\./zcc2')
if [ -z "$pid" ]; then
    echo "zcc2 not running"
    exit 1
fi
echo "Attaching to $pid"
gdb -batch -p $pid -ex 'bt' -ex 'info locals' -ex 'info registers rip rsp rbp' -ex 'x/10i $rip' -ex 'quit'
