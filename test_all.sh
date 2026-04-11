#!/bin/bash
PASS=0
FAIL=0
for f in fuzz_results_gods_eye/mismatches/*.c; do
  rm -f t_gcc.log t_zcc.log t_zcc.s
  gcc -O0 -w "$f" -o t_gcc
  ./t_gcc > t_gcc.log
  if ! ./zcc2 "$f" -o t_zcc.s >/dev/null 2>&1; then
      FAIL=$((FAIL+1))
      echo "FAIL (ZCC CRASH): $f"
      continue
  fi
  if ! gcc -O0 -w t_zcc.s -o t_zcc >/dev/null 2>&1; then
      FAIL=$((FAIL+1))
      echo "FAIL (ASM ERROR): $f"
      continue
  fi
  ./t_zcc > t_zcc.log
  if diff -q t_gcc.log t_zcc.log >/dev/null 2>&1; then
    PASS=$((PASS+1))
  else
    FAIL=$((FAIL+1))
    echo "FAIL (DIFF): $f"
  fi
done
echo "PASS: $PASS"
echo "FAIL: $FAIL"
