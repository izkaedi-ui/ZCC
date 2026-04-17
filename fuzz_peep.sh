#!/bin/bash
TOTAL=0
PASS=0
FAIL=0

make -f Makefile.release zcc

export CSMITH_PATH=$(find /usr/include -name csmith.h -printf '%h\n' -quit 2>/dev/null)
if [ -z "$CSMITH_PATH" ]; then
    CSMITH_PATH="/usr/include/csmith-2.3.0"
    if [ ! -d "$CSMITH_PATH" ]; then CSMITH_PATH="/usr/include/csmith"; fi
fi

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
#define platform_main_begin()
#define platform_main_end(crc, flag)
#define crc32_gentab()
#define transparent_crc(val, varname, flag)
#endif
FAKEOF

for i in $(seq 1 100); do
    csmith --no-packed-struct --no-unions --no-bitfields --no-volatiles --no-float \
           --no-const-pointers --no-volatile-pointers \
           --no-safe-math --no-checksum > test_p_$i.c
           
    sed -i -e 's/#include "csmith.h"/#include "fake_csmith.h"/g' \
           -e 's/volatile //g' \
           test_p_$i.c
           
    timeout 15 ./zcc test_p_$i.c --ir -o zcc_no_peep_$i.s > /dev/null 2>&1
    E1=$?
    if [ $E1 -ne 0 ]; then
        rm -f test_p_$i.c zcc_no_peep_$i.s
        continue
    fi
    gcc -o zcc_no_peep_$i zcc_no_peep_$i.s -lm
    
    timeout 15 ./zcc test_p_$i.c --ir --peephole --peephole-deterministic -o zcc_peep_$i.s > /dev/null 2>&1
    E2=$?
    if [ $E2 -ne 0 ]; then
        echo "Fail: Peep failed to compile but NO Peep passed ($i)"
        FAIL=$((FAIL+1))
        continue
    fi
    gcc -o zcc_peep_$i zcc_peep_$i.s -lm
    
    timeout 5 ./zcc_no_peep_$i > out_no_peep_$i.txt
    R1=$?
    timeout 5 ./zcc_peep_$i > out_peep_$i.txt
    R2=$?
    
    diff -u out_no_peep_$i.txt out_peep_$i.txt > diff_p_$i.txt
    if [ -s diff_p_$i.txt ] || [ $R1 -ne $R2 ]; then
        echo "Test $i: FAIL (No_Peep: $R1, Peep: $R2)"
        FAIL=$((FAIL+1))
    else
        PASS=$((PASS+1))
    fi
    TOTAL=$((TOTAL+1))
    rm -f test_p_$i.c zcc_no_peep_$i* zcc_peep_$i* out_no_peep_$i.txt out_peep_$i.txt diff_p_$i.txt
done
echo "PEEP VS NO-PEEP FUZZ: $PASS PASS / $FAIL FAIL / $TOTAL TOTAL"
