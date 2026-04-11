#!/bin/bash
rm -f fuzz_post_alloca.log
pass=0
fail=0
total=0
for f in fuzz_50/*.c; do
  total=$((total+1))
  gcc -O0 -w $f -o rgcc && ./rgcc > go.txt 2>/dev/null
  ./zcc $f -o rzcc.s 2>/dev/null && gcc -o rzcc rzcc.s && ./rzcc > zo.txt 2>/dev/null
  if diff -q go.txt zo.txt >/dev/null; then
    echo "$f: PASS"
    pass=$((pass+1))
  else
    echo "$f: FAIL"
    fail=$((fail+1))
  fi
done
echo "Score: $pass/$total"
