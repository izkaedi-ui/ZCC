#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

echo '=== Step through main in zcc3_ir ==='
echo 'int main(){return 0;}' > /tmp/t1.c

cat > /tmp/gdb4.txt << 'GDB'
set pagination off
set confirm off
break main
run /tmp/t1.c -o /tmp/t1.s

# Step 40 instructions from main entry and print each
set $count = 0
while $count < 60
  stepi
  set $count = $count + 1
  x/1i $pc
end
quit
GDB

gdb -batch -x /tmp/gdb4.txt ./zcc3_ir 2>&1 | head -150

echo ''
echo '=== For comparison: main in reference ==='
cat > /tmp/gdb4r.txt << 'GDB2'
set pagination off
set confirm off
break main
run /tmp/t1.c -o /tmp/t1_ref.s
set $count = 0
while $count < 40
  stepi
  set $count = $count + 1
  x/1i $pc
end
quit
GDB2

gdb -batch -x /tmp/gdb4r.txt ./zcc 2>&1 | head -100

echo DONE
