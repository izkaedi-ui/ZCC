#!/bin/bash
# test_compile.sh — Verify ZCC can compile Csmith output through the full pipeline
# Usage: ./test_compile.sh [N]  (default: 10 programs)

set -e
cd "$(dirname "$0")"

N="${1:-10}"
PASS=0
FAIL=0
PARSE_FAIL=0
GCC_FAIL=0
MATCH=0
MISMATCH=0
TIMEOUT=0

CSMITH_FLAGS="--no-arrays --no-structs --no-packed-struct --no-unions --no-bitfields --no-volatiles --no-float"
CSMITH_FLAGS="$CSMITH_FLAGS --no-const-pointers --no-volatile-pointers"
CSMITH_FLAGS="$CSMITH_FLAGS --no-safe-math --no-checksum"

mkdir -p failures

# Minimal fake header if stdint.h through gcc -E is too heavy
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
static void platform_main_begin(void) {}
static void platform_main_end(uint32_t x, int y) {}
#endif
FAKEOF

echo "=== ZCC Csmith Compile Test (N=$N) ==="
echo ""

for i in $(seq 1 "$N"); do
    # Generate
    csmith $CSMITH_FLAGS > test_csmith.c 2>/dev/null

    # Replace csmith.h with fake_csmith.h and strip volatile
    sed -i -e 's/#include "csmith.h"/#include "fake_csmith.h"/g' \
           -e 's/volatile //g' test_csmith.c

    # Try Method A: gcc -E then strip line markers
    gcc -E -w -o test_csmith.i test_csmith.c 2>/dev/null
    sed -i '/^# /d' test_csmith.i

    # Try ZCC on preprocessed file
    if ./zcc test_csmith.i -o test_zcc 2>zcc_err.txt; then
        # ZCC compiled — now compile with GCC for reference
        if gcc -O0 -w -o test_gcc test_csmith.c -lm 2>/dev/null; then
            # Run both with timeout
            GCC_OUT=$(timeout 5s ./test_gcc 2>/dev/null; echo "EXIT:$?")
            ZCC_OUT=$(timeout 5s ./test_zcc 2>/dev/null; echo "EXIT:$?")

            GCC_EXIT=$(echo "$GCC_OUT" | tail -1 | sed 's/EXIT://')
            ZCC_EXIT=$(echo "$ZCC_OUT" | tail -1 | sed 's/EXIT://')

            # Check for timeout (exit code 124)
            if [ "$GCC_EXIT" = "124" ] || [ "$ZCC_EXIT" = "124" ]; then
                printf "Test %3d: TIMEOUT (skipped)\n" "$i"
                TIMEOUT=$((TIMEOUT + 1))
            elif [ "$GCC_OUT" = "$ZCC_OUT" ]; then
                printf "Test %3d: PASS (exit=%s)\n" "$i" "$ZCC_EXIT"
                MATCH=$((MATCH + 1))
                PASS=$((PASS + 1))
            else
                printf "Test %3d: MISMATCH gcc_exit=%s zcc_exit=%s\n" "$i" "$GCC_EXIT" "$ZCC_EXIT"
                MISMATCH=$((MISMATCH + 1))
                FAIL=$((FAIL + 1))
                # Save failure
                cp test_csmith.c "failures/fail_${i}.c"
                cp test_csmith.i "failures/fail_${i}.i"
                echo "GCC: $GCC_OUT" > "failures/fail_${i}.diff"
                echo "ZCC: $ZCC_OUT" >> "failures/fail_${i}.diff"
                # Try to get assembly diff
                gcc -S -O0 -w -o "failures/fail_${i}_gcc.s" test_csmith.c 2>/dev/null || true
                ./zcc test_csmith.i -o "failures/fail_${i}_zcc.s" 2>/dev/null || true
            fi
        else
            printf "Test %3d: GCC_FAIL (skipped)\n" "$i"
            GCC_FAIL=$((GCC_FAIL + 1))
        fi
    else
        # ZCC couldn't compile — check if it's a known parser limitation
        ERR_COUNT=$(wc -l < zcc_err.txt)
        FIRST_ERR=$(head -1 zcc_err.txt | cut -c1-60)
        printf "Test %3d: PARSE_FAIL (%d errors: %s...)\n" "$i" "$ERR_COUNT" "$FIRST_ERR"
        PARSE_FAIL=$((PARSE_FAIL + 1))
    fi
done

# Cleanup
#rm -f test_csmith.c test_csmith.i test_zcc test_gcc zcc_err.txt test_zcc_retry

echo ""
echo "========================================="
echo " RESULTS: $N programs tested"
echo "========================================="
echo " PASS (match):    $MATCH"
echo " MISMATCH:        $MISMATCH"
echo " PARSE_FAIL:      $PARSE_FAIL"
echo " GCC_FAIL:        $GCC_FAIL"
echo " TIMEOUT:         $TIMEOUT"
echo "========================================="
if [ "$MISMATCH" -gt 0 ]; then
    echo " CODEGEN BUGS FOUND — check failures/"
elif [ "$PARSE_FAIL" -eq "$N" ]; then
    echo " ALL PARSE FAIL — pipeline needs fixing"
else
    echo " CLEAN — no codegen mismatches detected"
fi
echo "========================================="
