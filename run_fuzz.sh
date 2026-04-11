#!/bin/bash
PASS=0; FAIL=0; CRASH=0
for i in $(seq 1 500); do
    csmith --no-packed-struct --no-unions --no-bitfields > fuzz_$i.c
    gcc -E -P fuzz_$i.c -o fuzz_pp.c 2>/dev/null || continue
    ./zcc fuzz_pp.c -o fuzz.s 2>/dev/null || { CRASH=$((CRASH+1)); echo "COMPILE FAIL: fuzz_$i.c"; continue; }
    gcc -o fuzz_zcc fuzz.s -lm 2>/dev/null || { CRASH=$((CRASH+1)); echo "LINK FAIL: fuzz_$i.c"; continue; }
    gcc -O0 -o fuzz_gcc fuzz_$i.c -lm 2>/dev/null || continue
    ZCC_OUT=$(timeout 5 ./fuzz_zcc 2>/dev/null)
    GCC_OUT=$(timeout 5 ./fuzz_gcc 2>/dev/null)
    if [ "$ZCC_OUT" = "$GCC_OUT" ]; then
        PASS=$((PASS+1))
    else
        FAIL=$((FAIL+1))
        echo "MISMATCH $i: ZCC='$ZCC_OUT' GCC='$GCC_OUT'"
        cp fuzz_$i.c fuzz_mismatch_$i.c
    fi
    echo "Progress: $i/500 pass=$PASS fail=$FAIL crash=$CRASH"
done
echo "FINAL: pass=$PASS fail=$FAIL crash=$CRASH"
