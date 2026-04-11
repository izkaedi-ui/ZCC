PID=$(pidof zcc2)
if [ -z "$PID" ]; then
    echo "zcc2 not running"
    exit 1
fi
echo "PID is $PID"
gdb -batch -p $PID -ex "bt" -ex "info locals" -ex "info registers rip rsp rbp" -ex "x/10i \$rip" -ex "quit" > gdb_out.txt 2>&1
