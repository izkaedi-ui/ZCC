#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== GDB: trace zcc3_ir execution ==='
echo 'int main(){return 0;}' > /tmp/t1.c

# Create GDB command file
cat > /tmp/gdb_cmds.txt << 'GDB'
set pagination off
set confirm off

# Break at key functions
break read_file
break init_compiler
break next_token
break parse_program
break codegen_program
break peephole_optimize
break fopen
break exit

run /tmp/t1.c -o /tmp/t1.s

# If we hit a breakpoint, show where we are
bt 5
continue
bt 5
continue
bt 5
continue
bt 5
continue
bt 5
continue
bt 5
continue
bt 5
continue
bt 5
continue
bt 5
continue
bt 5
continue
quit
GDB

gdb -batch -x /tmp/gdb_cmds.txt ./zcc3_ir 2>&1 | head -80

echo ''
echo '=== Now compare: reference compiler ==='
cat > /tmp/gdb_ref.txt << 'GDB2'
set pagination off
set confirm off
break read_file
break exit
run /tmp/t1.c -o /tmp/t1_ref.s
bt 5
continue
bt 5
continue
bt 5
quit
GDB2

gdb -batch -x /tmp/gdb_ref.txt ./zcc 2>&1 | head -40

echo DONE
