#!/bin/bash
PASS=0
FAIL=0
for f in fuzz_results_gods_eye/mismatches/*.c; do
  gcc -O0 -w "$f" -o gcc_out
  ./gcc_out > gcc_out.log
  ./zcc2 "$f" -o zcc_out.s >/dev/null 2>&1
  gcc -O0 -w zcc_out.s -o zcc_out
  ./zcc_out > zcc_out.log
  if diff -q gcc_out.log zcc_out.log >/dev/null 2>&1; then
    PASS=$((PASS+1))
  else
    FAIL=$((FAIL+1))
    echo "FAIL: $f"
  fi
done
echo "PASS: $PASS"
echo "FAIL: $FAIL"
