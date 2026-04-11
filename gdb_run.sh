#!/usr/bin/env bash
# Start GDB with the binary, let it run, interrupt after 3s and get bt
echo "set pagination off
break whereLoopAddBtree
run
continue" | gdb -batch ./sqlite3_test > /tmp/gdb_out_start.txt 2>&1 &
GDB_PID=$!
sleep 5
kill -INT $GDB_PID 2>/dev/null
sleep 1
# Now attach to the already-running process if not dead
echo "done"
cat /tmp/gdb_out_start.txt
