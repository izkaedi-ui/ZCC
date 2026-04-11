#!/bin/sh
# simple_watch.sh — Catch stack corruption with GDB watchpoints
# Usage: wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && sh simple_watch.sh"

echo "int main() { return 42; }" > /tmp/tiny.c

# First: find main's RBP value
echo "=== Step 1: Getting main's frame pointer ==="
RBP=$(gdb -batch \
  -ex 'break main' \
  -ex 'run /tmp/tiny.c -o /tmp/tiny.s' \
  -ex 'next' \
  -ex 'next' \
  -ex 'print/x $rbp' \
  ./zcc2 2>&1 | grep '^\$1' | sed 's/.*= //')

echo "main RBP = $RBP"

if [ -z "$RBP" ]; then
  echo "Failed to get RBP. Trying alternate method..."
  gdb -batch \
    -ex 'break main' \
    -ex 'run /tmp/tiny.c -o /tmp/tiny.s' \
    -ex 'next' \
    -ex 'next' \
    -ex 'info registers rbp' \
    ./zcc2 2>&1 | tail -5
  exit 1
fi

# Calculate return address location (RBP + 8)
# Use Python to do hex math
RET_LOC=$(python3 -c "print(hex(int('$RBP', 16) + 8))")
echo "Return address at: $RET_LOC"

# Step 2: Set hardware watchpoint on the return address
echo ""
echo "=== Step 2: Watching return address for corruption ==="
gdb -batch \
  -ex 'break main' \
  -ex 'run /tmp/tiny.c -o /tmp/tiny.s' \
  -ex 'next' \
  -ex 'next' \
  -ex "watch *(long long*)$RET_LOC" \
  -ex 'continue' \
  -ex 'bt 10' \
  -ex 'info registers rip rbp rsp rax' \
  -ex 'x/3i $rip-8' \
  ./zcc2 2>&1 | tail -30
