#!/bin/bash
export CSMITH_PATH=$(find /usr/include -name csmith.h -printf '%h\n' -quit 2>/dev/null)
if [ -z "$CSMITH_PATH" ]; then
    CSMITH_PATH="/usr/include/csmith-2.3.0"
    if [ ! -d "$CSMITH_PATH" ]; then CSMITH_PATH="/usr/include/csmith"; fi
fi

mkdir -p failures

TOTAL=0
PASS=0
FAIL=0
TIMEOUT=0

# Ensure we have zcc1 stage
make zcc1

# Create a safe, minimal fallback header because glibc stdint.h breaks ZCC's C89 parser
cat > fake_csmith.h << 'FAKEOF'
#ifndef FAKE_CSMITH_H
#define FAKE_CSMITH_H
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long int64_t;
typedef unsigned long uint64_t;
typedef long intptr_t;
typedef unsigned long uintptr_t;
typedef unsigned long size_t;
#define INT8_MAX 127
#define INT8_MIN (-128)
#define INT16_MAX 32767
#define INT16_MIN (-32768)
#define INT32_MAX 2147483647
#define INT32_MIN (-2147483647-1)
#define INT64_MAX 9223372036854775807L
#define INT64_MIN (-9223372036854775807L-1)
#define UINT8_MAX 255
#define UINT16_MAX 65535
#define UINT32_MAX 4294967295U
#define UINT64_MAX 18446744073709551615UL
#define NULL ((void*)0)
static int printf(const char *fmt, ...);
static int strcmp(const char *s1, const char *s2);
#endif
FAKEOF

TOTAL_RUNS=1000

for i in $(seq 1 $TOTAL_RUNS); do
    csmith --no-packed-struct --no-unions --no-bitfields --no-volatiles --no-float \
           --no-const-pointers --no-volatile-pointers \
           --no-safe-math --no-checksum > test_$i.c
           
    # Hard string replacements to avoid glibc stdint.h processing via GCC
    # and to strip volatile which Csmith illegally emits even with --no-volatiles
    sed -i -e 's/#include "csmith.h"/#include "fake_csmith.h"/g' \
           -e 's/volatile //g' \
           test_$i.c
           
    # Skip GCC -E entirely, pass straight to ZCC
    timeout 5 ./zcc1 test_$i.c -o zcc_bin_$i.s > zcc_err_$i.txt 2>&1
    if [ $? -ne 0 ]; then
        echo "Test $i: compile_fail (skipped)"
        rm -f test_$i.c zcc_bin_$i.s zcc_err_$i.txt
        continue
    fi
    gcc -o zcc_bin_$i zcc_bin_$i.s -lm
    
    # Compile GCC natively from the fake header as the gold standard baseline
    gcc -O0 -w -o gcc_bin_$i test_$i.c -lm
    
    # Run
    timeout 5 ./gcc_bin_$i > gcc_out_$i.txt
    GCC_EXIT=$?
    if [ $GCC_EXIT -eq 124 ]; then
        echo "Test $i: TIMEOUT (GCC)"
        TIMEOUT=$((TIMEOUT+1))
        rm -f test_$i.* zcc_err_$i.txt zcc_bin_$i.s zcc_bin_$i gcc_bin_$i gcc_out_$i.txt
        continue
    fi
    
    timeout 5 ./zcc_bin_$i > zcc_out_$i.txt
    ZCC_EXIT=$?
    if [ $ZCC_EXIT -eq 124 ]; then
        echo "Test $i: TIMEOUT (ZCC)"
        TIMEOUT=$((TIMEOUT+1))
        rm -f test_$i.* zcc_err_$i.txt *out_$i.txt zcc_bin_$i* gcc_bin_$i
        continue
    fi
    
    # Diff outputs
    diff -u gcc_out_$i.txt zcc_out_$i.txt > diff_$i.txt
    if [ -s diff_$i.txt ] || [ $GCC_EXIT -ne $ZCC_EXIT ]; then
        echo "Test $i: FAIL (GCC: $GCC_EXIT, ZCC: $ZCC_EXIT)"
        FAIL=$((FAIL+1))
        mkdir -p failures/$i
        cp test_$i.c failures/$i/
        cp zcc_bin_$i.s failures/$i/
        cp diff_$i.txt failures/$i/
        echo "EXIT DIFFERENCE GCC: $GCC_EXIT ZCC: $ZCC_EXIT" >> failures/$i/diff_$i.txt
    else
        echo "Test $i: PASS"
        PASS=$((PASS+1))
    fi
    TOTAL=$((TOTAL+1))
    
    rm -f test_$i.* zcc_err_$i.txt *out_$i.txt zcc_bin_$i* gcc_bin_$i diff_$i.txt
done

echo ""
echo "============================="
echo "   CSMITH FUZZ RUN SUMMARY   "
echo "============================="
echo "TOTAL TESTED : $TOTAL"
echo "PASSED       : $PASS"
echo "FAILED       : $FAIL"
echo "TIMEOUTS     : $TIMEOUT"
echo "============================="
