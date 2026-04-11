#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux/fuzz_ir
FAIL=$(ls mismatches/*.c 2>/dev/null | head -1)
echo "=== MISMATCH SOURCE ==="
cat "$FAIL"
echo ""
echo "=== ZCC STDERR ==="
ZCC_IR_BACKEND=1 ./zcc_new "$FAIL" -o /tmp/test_out.s 2>&1 | head -40
