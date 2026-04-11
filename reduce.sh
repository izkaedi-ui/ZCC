#!/bin/bash
gcc -I. -E $1 | grep -v '^#' > /tmp/test.i
./zcc /tmp/test.i -o /tmp/test.s 2>/dev/null
gcc -o /tmp/zcc_bin /tmp/test.s -lm 2>/dev/null
/tmp/zcc_bin
ZCC_EX=$?
gcc -I. -O0 -o /tmp/gcc_bin $1 -lm 2>/dev/null
/tmp/gcc_bin
GCC_EX=$?
echo "ZCC=$ZCC_EX GCC=$GCC_EX"
