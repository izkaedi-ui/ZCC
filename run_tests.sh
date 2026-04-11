#!/bin/bash
mkdir -p tests
echo "Building ZCC..."
make zcc

PASS=0
FAIL=0

for t in tests/test_*.c; do
    ./zcc "$t" -o "${t%.c}.s" 2>/dev/null
    if [ $? -ne 0 ]; then
        echo "[FAIL] $t (compile error)"
        FAIL=$((FAIL+1))
        continue
    fi
    gcc -o "${t%.c}.exe" "${t%.c}.s" -lm 2>/dev/null
    if [ $? -ne 0 ]; then
        echo "[FAIL] $t (link error)"
        FAIL=$((FAIL+1))
        continue
    fi
    ./${t%.c}.exe > /dev/null
    if [ $? -eq 0 ]; then
        echo "[PASS] $t"
        PASS=$((PASS+1))
    else
        echo "[FAIL] $t (runtime error)"
        FAIL=$((FAIL+1))
    fi
done

echo ""
echo "Summary: $PASS passed, $FAIL failed."
exit $FAIL
