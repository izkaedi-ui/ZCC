#!/bin/bash
set -e
csmith --no-packed-struct --no-unions --no-bitfields --no-volatiles --no-float --no-const-pointers --no-volatile-pointers --no-safe-math --no-checksum > test_csmith.c
sed -i -e 's/#include "csmith.h"/#include "fake_csmith.h"/g' -e 's/volatile //g' test_csmith.c
gcc -E -w -o test_csmith.i test_csmith.c
sed -i '/^# /d' test_csmith.i
./zcc1 test_csmith.i -o test_zcc > err.txt 2>&1 || true
