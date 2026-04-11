#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux

echo "=== CRASH CASE ==="
CRASH=$(ls fuzz_ir/crashes/*.c 2>/dev/null | head -n 1)
if [ -z "$CRASH" ]; then
  echo "no .c in crashes dir, listing:"
  ls fuzz_ir/crashes/ | head -10
else
  cat "$CRASH"
  echo ""
  echo "--- ZCC stderr ---"
  ZCC_IR_BACKEND=1 ./zcc_new "$CRASH" -o /tmp/dbg.s 2>&1
fi

echo ""
echo "=== MISMATCH DIFF (first case) ==="
MM=$(ls fuzz_ir/mismatches/*.c 2>/dev/null | head -n 1)
if [ -n "$MM" ]; then
  BASE="${MM%.c}"
  gcc -O0 -w "$MM" -o /tmp/mm_gcc && /tmp/mm_gcc > /tmp/gcc_out.txt
  ZCC_IR_BACKEND=1 ./zcc_new "$MM" -o /tmp/mm_zcc.s 2>/dev/null && \
    gcc -O0 -w /tmp/mm_zcc.s -o /tmp/mm_zcc && /tmp/mm_zcc > /tmp/zcc_out.txt
  echo "GCC: $(cat /tmp/gcc_out.txt)"
  echo "ZCC: $(cat /tmp/zcc_out.txt)"
fi
