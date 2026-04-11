#!/bin/bash
csmith --no-packed-struct --no-unions --no-bitfields --no-volatiles --no-float \
       --no-const-pointers --no-volatile-pointers \
       --no-safe-math --no-checksum > test_error.c
       
sed -i -e 's/#include "csmith.h"/#include "fake_csmith.h"/g' \
       -e 's/volatile //g' \
       test_error.c

./zcc1 test_error.c -o test_error.s > zcc_error_log.txt 2>&1
cat zcc_error_log.txt | head -n 30
